#ifndef __DBG_H__
#define __DBG_H__

/*
	dbg.h
	���������� �������.
	� ������ ������ ��� �� ������� ���������.

	��� ���� ������� _sprintf �������� ������������:
	���� � �������� ��������� �� ������ ������������ NULL - ������������� ������ � UART.
	���� � �������� ��������� �� ������ ������������ -1 - ������������� ������ � OSD-����������.
	���� � �������� ��������� �� ������ ������������ -2 - ������������� ������ � OSD-����������.

*/

#if !defined( CORTEX )
	#include <stdio.h>
	#define _printf printf
	#define _sprintf sprintf
#endif

#define PRINTF_DEST_UART ((char*)0x00000000)
#define PRINTF_DEST_OSD  ((char*)0xFFFFFFFF)
#define PRINTF_DEST_LCD  ((char*)0xFFFFFFFE)
#define PRINTF_DEST_LCDF ((char*)0xFFFFFFFD)

int _printf(const char *format, ...);
int _sprintf(char *out, const char *format, ...);

#ifdef _DBGOUT
	#if defined( CORTEX )
		#define DBG	_printf
		#define ASSERT(x)	if(!(x)){DBG("\nAssertion '%s' failed in %s:%s#%d!\n", #x, __FILE__, __FUNCTION__, __LINE__); while(1);}
	#else
		#define DBG	printf
		#define ASSERT(x)	if(!(x)){DBG("\nAssertion '%s' failed in %s:%s#%d!\n", #x, __FILE__, __FUNCTION__, __LINE__); while(1);}
	#endif
#else
	#define DBG(x ...)
	#define ASSERT(x ...)
#endif

#endif
