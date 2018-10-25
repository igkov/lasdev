#ifndef __UART__
#define __UART__

#include <stdint.h>


#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

/* RS485 mode definition. */
#define RS485_NMMEN		(0x1<<0)
#define RS485_RXDIS		(0x1<<1)
#define RS485_AADEN		(0x1<<2)
#define RS485_SEL		(0x1<<3)
#define RS485_DCTRL		(0x1<<4)
#define RS485_OINV		(0x1<<5) 

int uart_init(uint32_t baudrate, uint32_t sys_clk);
void uart_irq(int en);
void uart_uininit(void);
void uart_putchar(const uint8_t ch);
uint32_t uart_getchar(void); //only interrupt

int uart_command(uint8_t *data, int len);
int uart_receive(void);
int uart_answer(uint8_t *data, int *len, int timeout_ms);
int uart_answer_check(void);

#endif // __UART__
