/**
	@file
	Реализация ГОСТ 28147-89 с оптимизацией под ARM7.

	Реализовано 2 режима:
	1) Простая замена. Блоки по 8 байт.
	2) Сцепление блоков ( x_(n+1) = x_(n+1) ^ E_k(x_n) ).
	   Используется для того, чтобы шифровать на уровне секторов и нельзя было пытаться 
	   заменять блоки по 8 байт в файловой системе на внешней памяти.

	@note
	Функции преобразования блока переписаны на асемблер, однако этого делать не стоило, 
	т.к. в данной реализации RV очень хорошо оптимизировал и в итоге Си-вариант работает 
	даже чуток быстрее.
	
	ГОСТ 28147-89
	
	Таблица замен: 
	 8 * 4 * 16 = 512 байт
	Развернутый ключ:
	 8 * 4 * 4  = 128 байт
	Шифруемый блок: 
	 2 * 4       = 8 байт (64 бит)
	Размер ключа:
	 32 байта (256 бит)
	
	Платформа: ARM7
	Компилятор: RealView MDK 3.x
	
	#defs:
	_ARM_   -  Использование ASM-эквивалентов. (!!!)
	           Эквивалентные функции не отлажены и работают некорректно.
	
	@c igorkov 2008

 */

#include "gost.h"

#if defined( CORTEX )
#define ZERO_INIT __attribute__((zero_init))
#else
#define ZERO_INIT 
#endif

// Расширенный ключ (ключевое расписание)
ZERO_INIT unsigned int SKEY[32];
// Буфер, в который кладется результат преобразования
ZERO_INIT unsigned int BLOCK[2];

// @note: 
// Оптимизировано под быстродействие (в ущерб требуемому объему)
// xxxx 0000 xxxx xxxx xxxx xxxx xxxx xxxx

// Номер S-блока  Значение 
// 1              4    10   9    2    13   8    0    14   6    11   1    12   7    15   5    3   
// 2              14   11   4    12   6    13   15   10   2    3    8    1    0    7    5    9   
// 3              5    8    1    13   10   3    4    2    14   15   12   7    6    0    9    11  
// 4              7    13   10   1    0    8    9    15   14   4    6    12   11   2    5    3   
// 5              6    12   7    1    5    15   13   8    4    10   9    14   0    3    11   2   
// 6              4    11   10   0    7    2    1    13   3    6    8    5    9    12   15   14  
// 7              13   11   4    1    3    15   5    9    0    10   14   7    6    8    2    12  
// 8              1    15   13   0    5    7    10   4    9    2    3    14   6    11   8    12  
#define SB(a,b)   ((unsigned int)(b << ((a-1) * 4)))                                      
#define S1(a)     SB(1, a)
#define S2(a)     SB(2, a)
#define S3(a)     SB(3, a)
#define S4(a)     SB(4, a)
#define S5(a)     SB(5, a)
#define S6(a)     SB(6, a)
#define S7(a)     SB(7, a)
#define S8(a)     SB(8, a)
/**
	Забавно, но при размещении таблице во FLASH падения скорости не оказалось 
	(или это только эмулятор).
 */

#if defined( SUBST_SMALL )

const unsigned char SUBST[8][16] = 
{
	{  4,   10,  9,   2,   13,  8,   0,   14,  6,   11,  1,   12,  7,   15,  5,   3   }, 
	{  14,  11,  4,   12,  6,   13,  15,  10,  2,   3,   8,   1,   0,   7,   5,   9   }, 
	{  5,   8,   1,   13,  10,  3,   4,   2,   14,  15,  12,  7,   6,   0,   9,   11  }, 
	{  7,   13,  10,  1,   0,   8,   9,   15,  14,  4,   6,   12,  11,  2,   5,   3   }, 
	{  6,   12,  7,   1,   5,   15,  13,  8,   4,   10,  9,   14,  0,   3,   11,  2   }, 
	{  4,   11,  10,  0,   7,   2,   1,   13,  3,   6,   8,   5,   9,   12,  15,  14  }, 
	{  13,  11,  4,   1,   3,   15,  5,   9,   0,   10,  14,  7,   6,   8,   2,   12  }, 
	{  1,   15,  13,  0,   5,   7,   10,  4,   9,   2,   3,   14,  6,   11,  8,   12  }, 
};

#else // SUBST_SMALL

const unsigned int SUBST[8][16] = 
{
	{ S1(4), S1(10),S1(9), S1(2), S1(13),S1(8), S1(0), S1(14),S1(6), S1(11),S1(1), S1(12),S1(7), S1(15),S1(5), S1(3),  }, 
	{ S2(14),S2(11),S2(4), S2(12),S2(6), S2(13),S2(15),S2(10),S2(2), S2(3), S2(8), S2(1), S2(0), S2(7), S2(5), S2(9),  }, 
	{ S3(5), S3(8), S3(1), S3(13),S3(10),S3(3), S3(4), S3(2), S3(14),S3(15),S3(12),S3(7), S3(6), S3(0), S3(9), S3(11), }, 
	{ S4(7), S4(13),S4(10),S4(1), S4(0), S4(8), S4(9), S4(15),S4(14),S4(4), S4(6), S4(12),S4(11),S4(2), S4(5), S4(3),  }, 
	{ S5(6), S5(12),S5(7), S5(1), S5(5), S5(15),S5(13),S5(8), S5(4), S5(10),S5(9), S5(14),S5(0), S5(3), S5(11),S5(2),  }, 
	{ S6(4), S6(11),S6(10),S6(0), S6(7), S6(2), S6(1), S6(13),S6(3), S6(6), S6(8), S6(5), S6(9), S6(12),S6(15),S6(14), }, 
	{ S7(13),S7(11),S7(4), S7(1), S7(3), S7(15),S7(5), S7(9), S7(0), S7(10),S7(14),S7(7), S7(6), S7(8), S7(2), S7(12), }, 
	{ S8(1), S8(15),S8(13),S8(0), S8(5), S8(7), S8(10),S8(4), S8(9), S8(2), S8(3), S8(14),S8(6), S8(11),S8(8), S8(12), }, 
};

#endif // SUBST_SMALL

void gost_init(uint32_t *key)
{
	int i;
	
	// Инициализация ключевого расписания
	// (k1, k2, k3, k4, k5, k6, k7, k8, k1, k2, k3, k4, k5, k6, k7, k8, k1, k2, k3, k4, k5, k6, k7, k8, k8, k7, k6, k5, k4, k3, k2, k1)
	for (i = 0; i < 24; i++)
	{
		SKEY[i] = key[i % 8];
	}
	for (     ; i < 32; i++)
	{
		SKEY[i] = key[31 - i];
	}
}

#if defined( SUBST_SMALL )

void gost_crypt_block(uint32_t x1, uint32_t x2)
{
	uint32_t a, b;
	int i;
	
	for (i = 0; i < 32; i++)
	{
		/**
			Функция усложнения
		 */
		a = (uint32_t)(x1 + SKEY[i]);
		
		b  = S1(SUBST[0][(a >> (32- 4))       ]);
		b |= S2(SUBST[1][(a >> (32- 8)) & 0x0F]);
		b |= S3(SUBST[2][(a >> (32-12)) & 0x0F]);
		b |= S4(SUBST[3][(a >> (32-16)) & 0x0F]);
		b |= S5(SUBST[4][(a >> (32-20)) & 0x0F]);
		b |= S6(SUBST[5][(a >> (32-24)) & 0x0F]);
		b |= S7(SUBST[6][(a >> (32-28)) & 0x0F]);
		b |= S8(SUBST[7][ a             & 0x0F]);
		
		b = (b << 11) | (b >> 21);
		b  = b ^ x2;
		/**
			Функция усложнения - Конец
		 */
		
		x2 = x1;
		x1 = b;
	}
	
	BLOCK[0] = x2;
	BLOCK[1] = x1;
	
	return;
}

void gost_decrypt_block(uint32_t x1, uint32_t x2)
{
	uint32_t a, b;
	int i;
	
	for (i = 31; i != -1; i--)
	{
		/**
			Функция усложнения
		 */
		a = (uint32_t)(x1 + SKEY[i]);
		
		b  = S1(SUBST[0][(a >> (32- 4))       ]);
		b |= S2(SUBST[1][(a >> (32- 8)) & 0x0F]);
		b |= S3(SUBST[2][(a >> (32-12)) & 0x0F]);
		b |= S4(SUBST[3][(a >> (32-16)) & 0x0F]);
		b |= S5(SUBST[4][(a >> (32-20)) & 0x0F]);
		b |= S6(SUBST[5][(a >> (32-24)) & 0x0F]);
		b |= S7(SUBST[6][(a >> (32-28)) & 0x0F]);
		b |= S8(SUBST[7][(a           ) & 0x0F]);
		
		b = (b << 11) | (b >> 21);
		b  = b ^ x2;
		/**
			Функция усложнения - Конец
		 */
		
		x2 = x1;
		x1 = b;
	}
	
	BLOCK[0] = x2;
	BLOCK[1] = x1;
	
	return;
}

#else // SUBST_SMALL

void gost_crypt_block(uint32_t x1, uint32_t x2)
{
	uint32_t a, b;
	int i;
	
	for (i = 0; i < 32; i++)
	{
		/** 
			Функция усложнения
		 */
		a = (uint32_t)(x1 + SKEY[i]);
		
		b  = SUBST[0][(a >> (32- 4))       ];
		b |= SUBST[1][(a >> (32- 8)) & 0x0F];
		b |= SUBST[2][(a >> (32-12)) & 0x0F];
		b |= SUBST[3][(a >> (32-16)) & 0x0F];
		b |= SUBST[4][(a >> (32-20)) & 0x0F];
		b |= SUBST[5][(a >> (32-24)) & 0x0F];
		b |= SUBST[6][(a >> (32-28)) & 0x0F];
		b |= SUBST[7][(a           ) & 0x0F];
		
		b = (b << 11) | (b >> 21);
		b  = b ^ x2;
		/**
			Функция усложнения - Конец
		 */
		
		x2 = x1;
		x1 = b;
	}
	
	BLOCK[0] = x2;
	BLOCK[1] = x1;
	
	return;
}

void gost_decrypt_block(uint32_t x1, uint32_t x2)
{
	uint32_t a, b;
	int i;
	
	for (i = 31; i != -1; i--)
	{
		/**
			Функция усложнения
		 */
		a = (uint32_t)(x1 + SKEY[i]);
		
		b  = SUBST[0][(a >> (32- 4))       ];
		b |= SUBST[1][(a >> (32- 8)) & 0x0F];
		b |= SUBST[2][(a >> (32-12)) & 0x0F];
		b |= SUBST[3][(a >> (32-16)) & 0x0F];
		b |= SUBST[4][(a >> (32-20)) & 0x0F];
		b |= SUBST[5][(a >> (32-24)) & 0x0F];
		b |= SUBST[6][(a >> (32-28)) & 0x0F];
		b |= SUBST[7][ a             & 0x0F];
		
		b = (b << 11) | (b >> 21);
		b  = b ^ x2;
		/**
			Функция усложнения - Конец
		 */
		
		x2 = x1;
		x1 = b;
	}
	
	BLOCK[0] = x2;
	BLOCK[1] = x1;
	
	return;
}

#endif // SUBST_SMALL

void gost_crypt_cbc(uint32_t *block, int len, uint32_t cbc1, uint32_t cbc2)
{
	int i;
	
	for (i = 0; i < len; i += 2)
	{
		if (i)
		{
			block[i+0] ^= BLOCK[0];
			block[i+1] ^= BLOCK[1];
		}
		else
		{
			block[i+0] ^= cbc1;
			block[i+1] ^= cbc2;
		}
		gost_crypt_block(block[i], block[i+1]);
		
		block[i+0]      = BLOCK[0];
		block[i+1]      = BLOCK[1];
	}
}

void gost_decrypt_cbc(uint32_t *block, int len, uint32_t cbc1, uint32_t cbc2)
{
	int i;
	uint32_t PRE_BLOCK[2];

	PRE_BLOCK[0] = cbc1;
	PRE_BLOCK[1] = cbc2;

	for (i = 0; i < len; i += 2)
	{
		gost_decrypt_block(block[i], block[i+1]);
		
		BLOCK[0]     ^= PRE_BLOCK[0];
		BLOCK[1]     ^= PRE_BLOCK[1];
		
		PRE_BLOCK[0]  = block[i+0];
		PRE_BLOCK[1]  = block[i+1];
		
		block[i+0]    = BLOCK[0];
		block[i+1]    = BLOCK[1];
	}
}
