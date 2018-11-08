#ifndef _GIO_LPC13XX_H_
#define _GIO_LPC13XX_H_

void gio_init(void);									// Инициализация.
void gio_out(unsigned char bPort, unsigned char bPin);	// Установка пина, как вывод.
void gio_in(unsigned char bPort, unsigned char bPin);	// Установка пина, как вход.
void gio_set0(unsigned char bPort, unsigned char bPin);	// Установка лог. 0 на пине.
void gio_set1(unsigned char bPort, unsigned char bPin);	// Установка лог. 1 на пине.
int  gio_get(unsigned char bPort, unsigned char bPin);	// Получаем значение на пине.

#endif //_GIO_LPC13XX_H_
