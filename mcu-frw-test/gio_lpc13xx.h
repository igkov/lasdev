#ifndef _GIO_LPC13XX_H_
#define _GIO_LPC13XX_H_

void gio_init(void);									// Инициализация.
void gio_out(unsigned char bPort, unsigned char bPin);	// Установка пина, как вывод.
void gio_in(unsigned char bPort, unsigned char bPin);	// Установка пина, как вход.
void gio_set0(unsigned char bPort, unsigned char bPin);	// Установка лог. 0 на пине.
void gio_set1(unsigned char bPort, unsigned char bPin);	// Установка лог. 1 на пине.
int  gio_get(unsigned char bPort, unsigned char bPin);	// Получаем значение на пине.

#endif //_GIO_LPC13XX_H_

#if 0

// НЕТ!
// Такая модель хоть и позволяет создать хороший HAL-уровень, но
// слишком массивна, да и идет потеря производительности на доступ 
// к пину ввода-вывода.

#define GIO_SPI_MISO	0x01
#define GIO_SPI_MOSI	0x02
#define GIO_SPI_SCK		0x03
#define GIO_SPI_CS		0x04

#define GIO_BOOT		0x10
#define GIO_UART_RX		0x11
#define GIO_UART_TX		0x12

#define GIO_IN_CH1		0x20
#define GIO_IN_CH2		0x21
#define GIO_IN_CH3		0x22
#define GIO_IN_CH4		0x23
#define GIO_IN_CH5		0x24

#define GIO_OUT_CH1		0x30
#define GIO_OUT_CH2		0x31
#define GIO_OUT_CH3		0x32
#define GIO_OUT_CH4		0x33

#define GIO_OSD_OUT		0x40
#define GIO_OSD_LINE	0x41
#define GIO_OSD_FRAME	0x42
#define GIO_OSD_ODD		0x43

void gio_dir(int pin);
void gio_get(int pin);
void gio_set(int pin);
void gio_clr(int pin);

#endif
