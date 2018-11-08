/*
	CDC Firmware
	
	igorkov / fsp@igorkov.org / 2018
 */

#include "LPC13xx.h"
#include "type.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "usb.h"
#include "usbcfg_LPC13xx.h"
#include "usbhw_LPC13xx.h"
#include "usbcore.h"
#include "cdc.h"
#include "cdcuser.h"

#include "gio_LPC13xx.h"
#include "fifo.h"
#include "uart.h"

extern fifo_t fifo_in;
extern fifo_t fifo_out;

volatile uint32_t msTicks;
void SysTick_Handler(void) {
	msTicks++;
}

void SysTick_Init(void) {
	msTicks = 0;
	if (SysTick_Config(SystemCoreClock / 1000)) {
		while (1);
	}
}

uint32_t get_ms_timer(void) {
	return msTicks;
}

extern unsigned char BulkBufIn  [USB_CDC_BUFSIZE]; // Buffer to store USB IN  packet
extern unsigned char BulkBufOut [USB_CDC_BUFSIZE]; // Buffer to store USB OUT packet

void cdc_data_send(void) {
	if (CDC_DepInEmpty) {
		int len;
		CDC_DepInEmpty = 0;
		len = fifo_avail_data(&fifo_out);
		//if (len == 0)
		//	return;
		if (len == USB_CDC_BUFSIZE)
			len = USB_CDC_BUFSIZE-1;
		else if (len > USB_CDC_BUFSIZE)
			len = USB_CDC_BUFSIZE;
		len = fifo_getn(&fifo_out, &BulkBufIn[0], len);
		USB_WriteEP(CDC_DEP_IN, &BulkBufIn[0], len);
	}
}

// Задержка на переходные процессы 
// (вызывается из Startup_LPC13xx.s):
void start_delay(void) {
	volatile int n;
	for (n = 0; n < 1000000; n++) { __NOP(); }
}

extern CDC_LINE_CODING CDC_LineCoding;

/*----------------------------------------------------------------------------
  Main Program
 *---------------------------------------------------------------------------*/
int main (void) {
	volatile int n;
	int ms;

	SystemCoreClockUpdate();                  // Clock setup
	SysTick_Init();                           // Event module init (1ms event)

	// @fix 2010.04.29
	// New Board, VBUS fix
	//gio_out(1, 11);
	//gio_set1(1, 11);

	// IO
	gio_init();
	// PWR:
	gio_out(0,1);
	gio_set1(0,1);
	// LED init:
	gio_out(2,9);
	gio_set1(2,9);

	USBIOClkConfig();                         // USB IO Config
	CDC_Init();                               // CDC Logic Initialization
	USB_Init();                               // USB Initialization
	USB_Connect(TRUE);                        // USB Connect

	while (!USB_Configuration) ;              // wait until USB is configured

	// Led On:
	gio_set0(2,9);

	// flush
	if (fifo_avail_data(&fifo_out)) {
		cdc_data_send();
	}

	// flush buffers
	fifo_flush(&fifo_out);
	fifo_flush(&fifo_in);

	// init uart
	uart_init(CDC_LineCoding.dwDTERate, SystemCoreClock);

	while (1) {
		uint8_t ch;

		// UART mode
		// if have data send:
		if (fifo_get(&fifo_in, &ch) == 0) {
			uart_putchar(ch);
		}

		// every 2ms send data:
		if ((get_ms_timer() - ms) > 2) {
			ms = get_ms_timer();
			// for out debug
			//fifo_put(&fifo_out, 'A');
			// send only 1 packet
			//while (fifo_avail_data(&fifo_out))
			if (fifo_avail_data(&fifo_out)) {
				cdc_data_send();
			}
		}
	}
}
