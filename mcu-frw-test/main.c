/**
	Тестовый проект на загрузку.
	Проверка функционирования логики загрузчика.
 */
#include "lpc13xx.h"
#include "leds.h"
#include "gio_lpc13xx.h"

extern void SoftwareReset(void);

void loader_entry(void)
{
	// 1. Uninit peripherals.
	// 2. Write flag (*(uint32_t*)0x10000000 = 0xF387E238).
	// 3. Software reset.

	int n;
	uint32_t *data = (uint32_t*)0x10000000;

	// Disable peripheral:
	//(*rom)->pUSBD->connect(FALSE);    // USB Disconnect
	SysTick->CTRL = 0;                  // Systick irq disable
	NVIC_DisableIRQ(ADC_IRQn);          // ADC irq disable
	NVIC_DisableIRQ(TIMER_32_0_IRQn);   // Timer0 irq disable
	NVIC_DisableIRQ(UART_IRQn);         // UART irq disable
	NVIC_DisableIRQ(USB_IRQn);          // USB irq disable

	*data = 0xF387E238;                 // Bootloader flag!

	for (n=0; n<1000000; n++) __NOP;
	SoftwareReset();
}

int main(void)
{
	int n;
	int state = 0;

	leds_init();

	while (1)
	{
		state ^= 1;
		
		led_red(state);
		led_green(state^1);
		
		for (n=0; n<1000000; n++) __NOP;
		
		if (gio_get(0, 1) == 0)
		{
			for (n=0; n<1000000; n++) __NOP;
			
			// boot low,
			// goto loader:
			loader_entry();
		}
	}

	return 0;
}
