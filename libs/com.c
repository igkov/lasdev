#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "com.h"

/*
	0 - Успешно
	1 - Ошибка открытия порта
	2 - Прочая ошибка (ошибка настройки)
 */
int com_init(pcom_struct_t com, int com_number, int baudrate)
{
	DCB dcb;
	COMMTIMEOUTS CommTimeouts;
	char acComName[20];
	// Обнуляем структуры:
	memset(com, 0x00, sizeof(com_struct_t));
	// Создаем имя устройства для открытия и открываем его:
	//sprintf(acComName, "com%d", com_number);
	sprintf(acComName, "\\\\.\\COM%d", com_number);
	com->hcom = (uint32_t)CreateFile(acComName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	// Проверяем успешность
	if((HANDLE)com->hcom == INVALID_HANDLE_VALUE)
	{
		printf("Can't open COM-Port %s! - Error: %ld\r\n", acComName, GetLastError());
		return 1;
	}
	printf("COM-Port %s opened...\n", acComName);
	// Получение статуса COM-порта и настройка параметров соединения.
	GetCommState((HANDLE)com->hcom, &dcb);
	dcb.BaudRate    = baudrate;
	dcb.ByteSize    = 8;
	dcb.StopBits    = ONESTOPBIT;
	dcb.Parity      = NOPARITY;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fOutX       = FALSE;
	dcb.fInX        = FALSE;
	dcb.fNull       = FALSE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	// Установка новых параметров соединения:
	if(SetCommState((HANDLE)com->hcom, &dcb) == 0)
	{
		printf("Can't set baudrate %d ! - Error: %ld\r\n", baudrate, GetLastError());
		return 2;
	}
	// ...
	if (SetCommMask((HANDLE)com->hcom, EV_RXCHAR | EV_TXEMPTY) == 0)
	{
		printf("SetCommMask() Error!\r\n");
		return 2;
	}
	// Настройка таймаутов чтения/записи:
	CommTimeouts.ReadIntervalTimeout         = 1;
	CommTimeouts.ReadTotalTimeoutMultiplier  = 0;
	CommTimeouts.ReadTotalTimeoutConstant    = 100; // 100ms
	CommTimeouts.WriteTotalTimeoutMultiplier = 0;
	CommTimeouts.WriteTotalTimeoutConstant   = 0;
	SetCommTimeouts((HANDLE)com->hcom, &CommTimeouts);
	// Очищаем буфера COM:
	if (PurgeComm((HANDLE)com->hcom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0)
		return 2;
	return 0;
}

int com_deinit(pcom_struct_t com)
{
	if (CloseHandle((HANDLE)com->hcom) == 0)
		return 1;
	return 0;
}

#if 1

int com_write(pcom_struct_t com, char *buff, int len)
{
	int ret;
	unsigned long realsize;
	ret = WriteFile((HANDLE)com->hcom, buff, len, &realsize, NULL);
	if (ret)
		return realsize;
	return 0;
}

int com_read(pcom_struct_t com, char *buff, int len)
{
	int ret;
	unsigned long realsize;
	ret = ReadFile((HANDLE)com->hcom, buff, len, &realsize, NULL);
	if (ret)
		return realsize;
	return 0;
}

#else

int com_putchar(pcom_struct_t com, char c)
{
	unsigned long realsize;
	if (WriteFile((HANDLE)com->hcom, &c, 1, &realsize, NULL) == 0)
		return 1;
	return 0;
}

int com_putblock(pcom_struct_t com, char *buff, int len)
{
	unsigned long realsize;
	unsigned long offset = 0;
	while (offset < len)
	{
		if (WriteFile((HANDLE)com->hcom, &buff[offset], len-offset, &realsize, NULL) == 0)
			return 1;
		offset += realsize;
	}
	return 0;
}

int com_control(pcom_struct_t com, unsigned char line)
{
	int ret;
	if(line & DTR_LINE)
		ret = EscapeCommFunction((HANDLE)com->hcom, SETDTR);
	else
		ret = EscapeCommFunction((HANDLE)com->hcom, CLRDTR);
	if(line & RTS_LINE)
		ret = EscapeCommFunction((HANDLE)com->hcom, SETRTS);
	else
		ret = EscapeCommFunction((HANDLE)com->hcom, CLRRTS);
	return ret;
}

int com_getblock(pcom_struct_t com, char *buff, unsigned char maxlen, unsigned long *realsize)
{
	unsigned long offset = 0;
	unsigned long timeout = 0;
	while (offset < maxlen)
	{
		ReadFile((HANDLE)com->hcom, &buff[offset], maxlen-offset, realsize, NULL);
		if (*realsize == 0)
		{
			timeout = 0;
		}
		else
		{
			timeout++;
			if (timeout > 100) // 10s wait
				break;
		}
		offset += *realsize;
	}
	*realsize = offset;
	if (offset != maxlen)
		return 1;
	return 0;
}

int com_getchar(pcom_struct_t com, char *ch)
{
	unsigned long realsize;
	return com_getblock(com, ch, 1, &realsize);
}

#endif