#ifndef __GOST_H__
#define __GOST_H__

#include <stdint.h>

/*
	gost_init()

	������������� �����.
	
	Param:
	key - ��������� �� ���� ����������, ������ ����� 32 �����.
	
	Return:
	none
 */
void gost_init(uint32_t *key);

/*
	gost_crypt_block()
	gost_decrypt_block()

	���������� ����-����� (8����).
	
	Param:
	x1, x2   - 1 � 2 ����� �����.
	
	Return:
	BLOCK[2] - ��������� ���������� �����.
 */
void gost_crypt_block(uint32_t x1, uint32_t x2);
void gost_decrypt_block(uint32_t x1, uint32_t x2);

/*
	gost_crypt_cbc()
	gost_decrypt_cbc()

	���������� ������� ������.
	���������� CBC. ������ 8 ������.
	
	Param:
	block      - �������� �� ����.
	len        - ������ ������ � uint32_t.
	cbc1, cbc2 - ����������������� �������� ��� ������� ����� �������.
	
	Return:
	none
 */
void gost_crypt_cbc(uint32_t *block, int len, uint32_t cbc1, uint32_t cbc2);
void gost_decrypt_cbc(uint32_t *block, int len, uint32_t cbc1, uint32_t cbc2);

#endif // __GOST_H__
