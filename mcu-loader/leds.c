#include <stdint.h>
#include <LPC13xx.H>
#include <core_cm3.h>

#include "leds.h"
#include "gio_lpc13xx.h"

#if defined( QFN33 )
	#define LED_G (1<<10)
#else
	#define LED_R (1<<3)
	#define LED_G (1<<9)
#endif

void leds_init(void)
{
#if defined( QFN33 )
	LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 6;
	LPC_GPIO1->DIR = LED_G;
#else
	LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 6;
	LPC_GPIO2->DIR = LED_R | LED_G;
#endif
}

void led_red(int state)
{
#if !defined( QFN33 )
	if (state)
		LPC_GPIO2->DATA &= ~LED_R;
	else
		LPC_GPIO2->DATA |=  LED_R;
#endif
}

void led_green(int state)
{
#if defined( QFN33 )
	if (state)
		LPC_GPIO1->DATA &= ~LED_G;
	else
		LPC_GPIO1->DATA |=  LED_G;
#else
	if (state)
		LPC_GPIO2->DATA &= ~LED_G;
	else
		LPC_GPIO2->DATA |=  LED_G;
#endif
}
