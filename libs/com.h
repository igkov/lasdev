#ifndef __COM_H__
#define __COM_H__

#include <stdint.h>

typedef struct
{
	uint32_t hcom; //HANDLE hcom;
	uint32_t res;
} com_struct_t, *pcom_struct_t;

#define DTR_LINE 1
#define RTS_LINE 2

int com_init(pcom_struct_t com, int com_number, int baudrate);
int com_deinit(pcom_struct_t com);
#if 1
int com_write(pcom_struct_t com, char *buff, int len);
int com_read(pcom_struct_t com, char *buff, int len);
#else
int com_putchar(pcom_struct_t com, char c);
int com_putblock(pcom_struct_t com, char *buff, int len);
int com_putstr(pcom_struct_t com, char *str);
int com_control(pcom_struct_t com, unsigned char line);
int com_getblock(pcom_struct_t com, char *buff, unsigned char maxlen, unsigned long *realsize);
int com_getchar(pcom_struct_t com, char *ch);
#endif

#endif //__COM_H__
