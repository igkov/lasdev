#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comn.h"

/*
	0 - Успешно
	1 - Ошибка открытия порта
	2 - Прочая ошибка (ошибка настройки)
 */
int com_init(pcom_struct_t com, int com_number, DWORD baudrate)
{
#if defined( WINDOWS )
	DCB    dcb;
	COMMTIMEOUTS CommTimeouts;
	char acComName[20];

	memset(com, 0x00, sizeof(com_struct_t));
	
	com->com_number = com_number;
	com->baudrate = baudrate;

	// Создаем имя устройства для открытия и открываем его:
	sprintf(acComName, "\\\\.\\COM%d", com_number);
	com->hcom = CreateFile(acComName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// Проверяем успешность
	if(com->hcom == INVALID_HANDLE_VALUE)
	{
		printf("Can't open COM-Port %s ! - Error: %ld\n", acComName, GetLastError());
		return 1;
	}
	printf("COM-Port %s opened...\n", com);

	// Получение статуса COM-порта и настройка параметров соединения.
	GetCommState(com->hcom, &dcb);
	dcb.BaudRate    = com->baudrate;
	dcb.ByteSize    = 8;
	dcb.StopBits    = ONESTOPBIT;
	dcb.Parity      = NOPARITY;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fOutX       = FALSE;
	dcb.fInX        = FALSE;
	dcb.fNull       = FALSE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	// Установка новых параметров соединения:
	if(SetCommState(com->hcom, &dcb) == 0)
	{
		printf("Can't set baudrate %d ! - Error: %ld", com->baudrate, GetLastError());
		return 2;
	}

	// ...
	if (SetCommMask(com->hcom, EV_RXCHAR | EV_TXEMPTY) == 0)
		return 2;
	
	// Настройка таймаутов чтения/записи:
	CommTimeouts.ReadIntervalTimeout         =    1;
	CommTimeouts.ReadTotalTimeoutMultiplier  =    0;
	CommTimeouts.ReadTotalTimeoutConstant    =    100;
	CommTimeouts.WriteTotalTimeoutMultiplier =    0;
	CommTimeouts.WriteTotalTimeoutConstant   =    0;
	SetCommTimeouts(com->hcom, &CommTimeouts);

	// Очищаем буфера COM:
	if (PurgeComm(com->hcom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0)
		return 2;
#else
	#error
#endif

	return 0;
}

/*
	0 - Все в порядке
	1 - Ошибка закрытия хендла
 */
int com_deinit(pcom_struct_t com)
{
#if defined( WINDOWS )
	if (CloseHandle(com->hcom) == 0)
		return 1;
#else
	#error
#endif
	return 0;
}

int com_putchar(pcom_struct_t com, char c)
{
#if defined( WINDOWS )
	unsigned long realsize;
	if (WriteFile(com->hcom, &c, 1, &realsize, NULL) == 0)
		return 1;
#else
	#error
#endif

	return 0;
}

int com_putblock(pcom_struct_t com, char *buff, int len)
{
#if defined( WINDOWS )
	unsigned long realsize;
	unsigned long offset = 0;

	while (offset < len)
	{
		if (WriteFile(com->hcom, &buff[offset], len-offset, &realsize, NULL) == 0)
			return 1;
		offset += realsize;
	}
#else
	#error
#endif
	return 0;
}

int com_putstr(pcom_struct_t com, char *str)
{
	return com_putblock(com, str, strlen(str));
}

int com_control(pcom_struct_t com, unsigned char line)
{
	int ret;
#if defined( WINDOWS )
	if(line & DTR_LINE)
		ret = EscapeCommFunction(com->hcom, SETDTR);
	else
		ret = EscapeCommFunction(com->hcom, CLRDTR);

	if(line & RTS_LINE)
		ret = EscapeCommFunction(com->hcom, SETRTS);
	else
		ret = EscapeCommFunction(com->hcom, CLRRTS);
#else
	#error
#endif
	return ret;
}

//static 

int com_getblock(pcom_struct_t com, char *buff, unsigned char maxlen, unsigned long *realsize)
{
#if defined( WINDOWS )
	unsigned long offset = 0;
	unsigned long timeout = 0;

	while (offset < maxlen)
	{
		ReadFile(com->hcom, &buff[offset], maxlen-offset, realsize, NULL);
		if (*realsize == 0)
		{
			timeout = 0;
		}
		else
		{
			timeout++;
			if (timeout > 100)
				break;
		}
		offset += *realsize;
	}

	*realsize = offset;

	if (offset != maxlen)
		return 1;
#else
	#error
#endif

	return 0;
}

int com_getchar(pcom_struct_t com, char *ch)
{
	unsigned long realsize;
	return com_getblock(com, ch, 1, &realsize);
}
