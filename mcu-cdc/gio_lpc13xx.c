/** 
	gio_lpc134x.c 

	Логика не полная. Пока корректно обрабатывается только PORT0.
	PORT1 обрабатывается корректно в случае, если он предварительно 
	настроен как GPIO, а не в режиме отладочного интерфейса.

	igorkov / 2011
 */
#include <lpc13xx.h>
#include "type.h"
#include "gio_lpc13xx.h"

static LPC_GPIO_TypeDef (* const LPC_GPIO[4]) = { LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3 };

void gio_out(unsigned char bPort, unsigned char bPin) {
	LPC_GPIO[bPort]->DIR |= 1<<bPin;
}

void gio_in(unsigned char bPort, unsigned char bPin) {
	LPC_GPIO[bPort]->DIR &= ~(1<<bPin);
}

void gio_set0(unsigned char bPort, unsigned char bPin) {
	LPC_GPIO[bPort]->MASKED_ACCESS[(1<<bPin)] = (0<<bPin);
}

void gio_set1(unsigned char bPort, unsigned char bPin)
{
	LPC_GPIO[bPort]->MASKED_ACCESS[(1<<bPin)] = (1<<bPin);
}

int  gio_get(unsigned char bPort, unsigned char bPin) {
	return LPC_GPIO[bPort]->MASKED_ACCESS[(1<<bPin)] ? 1 : 0;
}

void gio_init(void) {
	/* Enable AHB clock to the GPIO domain. */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6); 

#ifdef __JTAG_DISABLED  
	LPC_IOCON->JTAG_TDO_PIO1_1  &= ~0x07;
	LPC_IOCON->JTAG_TDO_PIO1_1  |= 0x01;
#endif 
}
