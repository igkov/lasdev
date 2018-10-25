#include <LPC13xx.H>
#include <core_cm3.h>
#include <stdint.h>

#include "spi.h"
#include "dbg.h"

#include "gio_lpc13xx.h"

#define INIT_CS()		gio_out(0,2)
#define CS(a)           gio_set##a(0,2)

void spi_delay(int cnt)
{
	int i;
	// 100kb/s
	// 100000 * 8 = 800kbit/s
	// delay = 1 / (2 * 800k) = 0.625us
	// 1 / 72000000 = 0.01388(8)us
	// ~ 45 циклов
	for (i=0;i<20*cnt;i++) __nop();
}

void spi_init(void)
{
	uint8_t i;

	INIT_CS();
	CS(1);

	LPC_SYSCON->PRESETCTRL      |= (0x1<<0);
	LPC_SYSCON->SSP0CLKDIV        = 0x02;		/* Divided by 2 */

	LPC_IOCON->PIO0_8            = 0x01;		/* SSP I/O config */
	LPC_IOCON->PIO0_9            = 0x01;		

	/* On HummingBird 1(HB1), SSP CLK can be routed to different pins, 
		other than JTAG TCK, it's either P2.11 func. 1 or P0.6 func. 2. */
	LPC_IOCON->SCK_LOC = 0x01;
	/* P2.11 function 1 is SSP clock, need to combined 
	   with IOCONSCKLOC register setting */
	LPC_IOCON->PIO2_11 = 0x01;

	/* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15 */
	LPC_SSP0->CR0 = 0x0707;

	/* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
	LPC_SSP0->CPSR = 0x04; // 0x08

	/* Enable SPI */
	LPC_SSP0->CR1 = SSPCR1_SSE;

	/* Send 20 spi commands with card not selected */
	for (i = 0; i < 21; i++)
	{
		spi_send(0xff);
	}
}

void spi_select(void)
{
	CS(0); //gio_set0(0,2); // ~CS
}

void spi_unselect(void)
{
	CS(1); //gio_set1(0,2); // ~CS
	spi_delay(100);
}

void spi_8bitmode(void)
{
	/* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15 */
	LPC_SSP0->CR0 = 0x0707;
}

void spi_9bitmode(void)
{
	/* Set DSS data to 9-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15 */
	LPC_SSP0->CR0 = 0x0708;
}

uint8_t spi_send(uint8_t bSend)

{
//	while ( (LPC_SSP->SR & (SSPSR_TNF|SSPSR_BSY)) != SSPSR_TNF );
	LPC_SSP0->DR = bSend;
	while ( (LPC_SSP0->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE );
	return LPC_SSP0->DR;
}
