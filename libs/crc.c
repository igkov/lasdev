#include "crc.h"

/*
	Name  : CRC-8
	Poly  : 0x31    x^8 + x^5 + x^4 + 1
	Init  : 0xFF
	Revert: false
	XorOut: 0x00
	Check : 0xF7 ("123456789")
	MaxLen: 15 байт(127 бит) - обнаружение одинарных, двойных, тройных и всех нечетных ошибок
 */
uint8_t crc8(uint8_t *block, uint16_t len)
{
	unsigned char crc = 0xFF;
	unsigned int i;
	while (len--)
	{
		crc ^= *block++;
		for (i = 0; i < 8; i++)
			crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : crc << 1;
	}
	return crc;
}

/*
	Name  : CRC-16 CCITT
	Poly  : 0x11021	x^16 + x^12 + x^5 + 1
	Init  : 0xFFFF
	Revert: false
	XorOut: 0x0000
	Check : 0x29B1 ("123456789")
	MaxLen: 4095 байт (32767 бит) - обнаружение одинарных, двойных, тройных и всех нечетных ошибок
 */
uint16_t crc16(uint8_t *block, uint16_t len)
{
	unsigned short crc = 0xFFFF;
	unsigned char i;
	while(len--)
	{
		crc ^= *block++ << 8;
		for (i = 0; i < 8; i++)
		{
			crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
		}
	}
	return crc;
}

#define W_GX    0x8411
#define DW_GX   0xDEBB20E3

uint32_t crc32b(uint32_t crc, uint8_t b)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if (crc & 0x80000000)
			crc ^= DW_GX;
		crc <<= 1;
		if (b & 0x80)
			crc++;
		b <<= 1;
	}
	return crc ;
}

uint32_t crc32(uint32_t crc, uint8_t *buff, uint32_t len )
{
	for ( ; len != 0; len--)
	{
		crc = crc32b( crc, *buff );
		buff++;
	}
	return( crc );
}
