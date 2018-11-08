#include <LPC13xx.H>
#include <stdint.h>
#include <string.h>

#include "uart.h"
#include "fifo.h"

extern fifo_t fifo_out;

void UART_IRQHandler(void) {
	uint8_t ch;
	uint8_t IIR_val = (LPC_UART->IIR >> 1) & 0x07;
	if (IIR_val == 0x03) {
		if (LPC_UART->LSR & LSR_RDR) {
			ch = LPC_UART->RBR;
		} else {
			return;
		}
    }
	else if (IIR_val == 0x02) {
		ch = LPC_UART->RBR;
	} else {
		return;
	}
	fifo_put(&fifo_out, ch);
	return;
}

int uart_init(uint32_t baudrate, uint32_t sys_clk) {
	int ret = 0;
	uint32_t Fdiv;
	NVIC_DisableIRQ(UART_IRQn);
	LPC_IOCON->PIO1_6 &= ~0x07;           /* UART I/O config */
	LPC_IOCON->PIO1_6 |=  0x01;           /* UART RXD */
	LPC_IOCON->PIO1_7 &= ~0x07;           /* ... */
	LPC_IOCON->PIO1_7 |=  0x01;           /* UART TXD */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12); /* Enable UART clock */
	LPC_SYSCON->UARTCLKDIV = 0x01;        /* Divided by 1 */
	LPC_UART->FDR          = 0x10;        /* DIVADDVAL|MULVAL, opt */
	LPC_UART->LCR          = 0x83;        /* 8 bits, no Parity, 1 Stop bit */
	Fdiv = (((sys_clk/LPC_SYSCON->SYSAHBCLKDIV)/LPC_SYSCON->UARTCLKDIV)/16)/baudrate;  /* Baud Rate */
	LPC_UART->DLM = Fdiv / 256;           /* ... */
	LPC_UART->DLL = Fdiv % 256;           /* ... */
	LPC_UART->LCR = 0x03;                 /* DLAB = 0 */
	LPC_UART->FCR = 0x07; /* Enable and reset TX and RX FIFO. */
	LPC_UART->IER = 0x01 | 0x04; // UART Recieve interrupt
	NVIC_EnableIRQ(UART_IRQn);
	return ret;
}

int uart_init_speed(uint32_t baudrate, uint32_t sys_clk) {
	uint32_t Fdiv;
	Fdiv = (((sys_clk/LPC_SYSCON->SYSAHBCLKDIV)/LPC_SYSCON->UARTCLKDIV)/16)/baudrate;  /* Baud Rate */
	LPC_UART->DLM = Fdiv / 256;           /* ... */
	LPC_UART->DLL = Fdiv % 256;           /* ... */
	return 0;
}

void uart_uninit(void) {
	LPC_IOCON->PIO1_6 &= ~ (0x07 | 0x18);
	LPC_IOCON->PIO1_7 &= ~ (0x07 | 0x18);
	LPC_GPIO1->DIR    &= ~((1<<6)|(1<<7));
}

void uart_putchar(const uint8_t ch) {
	while ( !(LPC_UART->LSR & LSR_THRE) );
	LPC_UART->THR = ch;
}

int uart_getchar(void) {
	while ( !(LPC_UART->LSR & LSR_RDR) );
	return LPC_UART->RBR;
}
