#include "lpc13xx.h"
#include "gio_lpc13xx.h"

#include "spi9.h"
	
#if 0
// SCLK - RXD/PIO1.6
// SDA  - TXD/PIO1.7
// CS   - BOOT/PIO0.1

#define SCLK_init 	gio_out(1,6)
#define SDA_init	gio_out(1,7)
#define CS_init 	gio_out(0,1)

#define SCLK_on 	gio_set1(1,6)
#define SDA_on 		gio_set1(1,7)
#define CS_on 		gio_set1(0,1)
#define SCLK_off 	gio_set0(1,6)
#define SDA_off 	gio_set0(1,7)
#define CS_off 		gio_set0(0,1)
#else
// SCLK - PIO3.1
// SDA  - PIO3.0
// CS   - PIO3.2

#define SCLK_init 	gio_out(3,1)
#define SDA_init	gio_out(3,0)
#define CS_init 	gio_out(3,2)

#define SCLK_on 	gio_set1(3,1)
#define SDA_on 		gio_set1(3,0)
#define CS_on 		gio_set1(3,2)
#define SCLK_off 	gio_set0(3,1)
#define SDA_off 	gio_set0(3,0)
#define CS_off 		gio_set0(3,2)
#endif

static void delay(unsigned int p)
{
	unsigned int i;
	
	for(i = 0; i < p; i++)
	{
		__NOP();
	}
}

void spi9_select(void)
{
	CS_off;
}

void spi9_unselect(void)
{
	SCLK_off;
	SDA_off;
	CS_on;
}

void spi9_send(uint16_t data)
{
	char i;

	for (i = 0; i < 9; i++)
	{
		SCLK_off;
		if(data & 0x100)
		{
			SDA_on;
		}
		else
		{
			SDA_off;
		}
		SCLK_on;
		data <<= 1;
		delay(2);
	}
}

void spi9_init(void)
{
	SCLK_init;
	SDA_init;
	CS_init;

	//SCLK_on;
	//SDA_on;
	SCLK_off;
	SDA_off;
	CS_off;
}
