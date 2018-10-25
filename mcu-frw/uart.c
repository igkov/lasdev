#include <LPC13xx.H>
#include <stdint.h>
#include <string.h>

#include "uart.h"
#include "event.h"

int uart_offset = 0;
uint8_t uart_status = 3; // 0 - new data, 1 - work, 2 - no data, 3 - proto error (proto, format, crc), 4 - no answer, timeout,
//uint8_t uart_data[128];
//uint8_t uart_data_len;
uint8_t uart_ch[2];

int Dist = -1;
int DistT = 0;
const char FindStr[] = { 'D', 'i', 's', 't', ':' };	//Dist: 625,
#if defined( BLUETOOTH )
const char CheckBTStr[] = { ':', 'B', 'T', 'M', 'O', 'D', 'E' };
static uint8_t OffsetBT = 0;
#endif

void UART_IRQHandler(void)
{
	uint8_t ch;
	uint8_t IIR_val = (LPC_UART->IIR >> 1) & 0x07;

	if (IIR_val == 0x03)
	{
		if (LPC_UART->LSR & LSR_RDR)	/* Receive Data Ready */
		{
			ch = LPC_UART->RBR;
		}
		else
		{
			return;
		}
    }
	else if (IIR_val == 0x02)
	{
		ch = LPC_UART->RBR;
	}
	else
	{
		return;
	}

#if defined( BLUETOOTH )
	if (CheckBTStr[OffsetBT] == ch)
	{
		OffsetBT++;
		if (OffsetBT == sizeof(CheckBTStr))
		{
			#define MODE_BT 0x03
			extern uint8_t mode;
			mode = MODE_BT;
			return;
		}
	}
	else
	{
		OffsetBT = 0;
	}
#endif

#if 1
	if (uart_offset < sizeof(FindStr))
	{
		if (FindStr[uart_offset] == ch)
		{
			uart_offset++;
		}
		else
		{
			uart_offset = 0;
			DistT = 0;
		}
	}
	else
	{
		if ((ch >= '0') && (ch <= '9'))
		{
			DistT *= 10;
			DistT += (ch - '0');
		}
		else if (ch == ' ')
		{
			//nop;
		}
		else if (ch == ',')
		{
			uart_offset = 0;
			Dist = DistT;
			DistT = 0;
		}
		else
		{
			uart_offset = 0;
			DistT = 0;
			Dist = -2;
		}
	}
#else
	//new data in ch
	if (uart_status == 0 || 
		uart_status == 3 || 
		uart_status == 4)
	{
		// ignore
	}
	else if (ch == '*')
	{
		// Ќовые данные ответа.
		uart_offset = 0;
		uart_status = 1;
		uart_data_len = 0;
	}
	else if (uart_status == 2)
	{
		// ignore
	}
	else if (ch == '#')
	{
		int crc = 0;
		int i;
		if (uart_status != 1)
		{
			uart_status = 3;
			return;
		}
		for (i = 0; i < uart_data_len-1; i++)
		{
			crc += uart_data[i];
		}
		if ((crc%100) != uart_data[uart_data_len-1])
		{
			uart_status = 3;
			return;
		}
		// все проверки пройдены, новые данные!
		uart_status = 0;
	}
	else if (ch >= '0' && ch <= '9')
	{
		if (uart_status != 1)
		{
			uart_status = 3;
			return;
		}
		uart_ch[uart_offset&1] = ch;
		if (uart_offset&1)
		{
			if (uart_data_len == sizeof(uart_data))
			{
				uart_status = 3;
				return;
			}
			uart_data[uart_data_len] = (uart_ch[0] - '0')*10 + (uart_ch[1] - '0');
			uart_data_len++;
		}
		uart_offset++;
	}
	else
	{
		if (uart_status == 1)
			uart_status = 3;
	}
#endif
	return;
}

int uart_init(uint32_t baudrate, uint32_t sys_clk)
{
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
	// TODO: –аскомментировать и проверить работоспособность!!!
	//LPC_UART->FCR = 0x07; /* Enable and reset TX and RX FIFO. */

#if 1
	// UART Interrupts:
	LPC_UART->IER = 0x01 | 0x04; // UART Recieve interrupt
	NVIC_EnableIRQ(UART_IRQn);
#endif

	return 0;
}

void uart_irq(int en)
{
	if (en)
	{
		LPC_UART->IER = 0x01 | 0x04;
		NVIC_EnableIRQ(UART_IRQn);
	}
	else
	{
		LPC_UART->IER = 0x00;
		NVIC_DisableIRQ(UART_IRQn);
	}
}

void uart_uninit(void)
{
	LPC_IOCON->PIO1_6 &= ~ (0x07 | 0x18);
	LPC_IOCON->PIO1_7 &= ~ (0x07 | 0x18);
	LPC_GPIO1->DIR    &= ~((1<<6)|(1<<7));
}

void uart_putchar(const uint8_t ch)
{
	while ( !(LPC_UART->LSR & LSR_THRE) );
	LPC_UART->THR = ch;
}

uint32_t uart_getchar(void)
{
	while ( !(LPC_UART->LSR & LSR_RDR) );
	return LPC_UART->RBR;
}

int uart_command(uint8_t *data, int len)
{
	int i;
	uart_status = 2;
	for (i = 0; i < len; i++)
		uart_putchar(data[i]);
	return 0;
}

int uart_receive(void)
{
	uart_status = 2;
	return 0;
}

int uart_answer_check(void)
{
	return uart_status;
}

#if 0
int uart_answer(uint8_t *data, int *len, int timeout_ms)
{
	uint32_t start = get_ms_timer();
	uint8_t ret;
	
	while (uart_status == 1 ||
		   uart_status == 2)
	{
		if ((get_ms_timer() - start) > timeout_ms)
		{
			return 4;
		}
	}
	
	if (uart_data_len > *len)
	{
		return 3;
	}
	
	memcpy(data, uart_data, uart_data_len);
	*len = uart_data_len;

	ret = uart_status;
	uart_status = 2;
	return ret;
}
#endif

