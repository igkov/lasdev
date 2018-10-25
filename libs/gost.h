#ifndef __GOST_H__
#define __GOST_H__

#include <stdint.h>

/*
	gost_init()

	Инициализация ключа.
	
	Param:
	key - Указатель на ключ шифрования, размер ключа 32 байта.
	
	Return:
	none
 */
void gost_init(uint32_t *key);

/*
	gost_crypt_block()
	gost_decrypt_block()

	Шифрование ГОСТ-блока (8байт).
	
	Param:
	x1, x2   - 1 и 2 слова блока.
	
	Return:
	BLOCK[2] - результат шифрования блока.
 */
void gost_crypt_block(uint32_t x1, uint32_t x2);
void gost_decrypt_block(uint32_t x1, uint32_t x2);

/*
	gost_crypt_cbc()
	gost_decrypt_cbc()

	Шифрование цепочки блоков.
	Связывание CBC. Кратно 8 байтам.
	
	Param:
	block      - указатеь на блок.
	len        - размер данных в uint32_t.
	cbc1, cbc2 - инициализационное значение для первого блока цепочки.
	
	Return:
	none
 */
void gost_crypt_cbc(uint32_t *block, int len, uint32_t cbc1, uint32_t cbc2);
void gost_decrypt_cbc(uint32_t *block, int len, uint32_t cbc1, uint32_t cbc2);

#endif // __GOST_H__
