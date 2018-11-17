#include <string.h>
#include <math.h>
#include "lpc13xx.h"
#include "type.h"

#include "usb.h"
#include "usbdesc.h"
#include "usbrom.h"

#include "io.h"
#include "config.h"

#include "crc.h"
#include "private_key.h"

#include "iap.h"
#include "gio_lpc13xx.h"
#include "gost.h"

extern uint32_t SystemFrequency;

extern void ASM_Goto(uint32_t addr);
extern void ASM_GotoSP(uint32_t addr, uint32_t sp);
const pimg_bin_t header = (pimg_bin_t)0x00001000;

ZERO_INIT USB_DEV_INFO DeviceInfo;
ZERO_INIT HID_DEVICE_INFO HidDevInfo;
ROM ** rom = (ROM **)0x1fff1ff8;

// RAM:
// 0x10000000 - System Flag (loader entry flag).
// 0x10000004 - Mode value.
// 0x10000008 - Serial Number.

uint32_t *sys_flag = (uint32_t*)0x10000000;

//int mode = 0; // 0 - loader, 1 - module
//uint32_t *mode     = (uint32_t*)0x10000004;
#define mode (*(uint32_t*)0x10000004)

uint32_t *sn       = (uint32_t*)0x10000008;


extern uint32_t __sn;

void NMI_Handler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.NMI_Handler);
}

void HardFault_Handler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.HardFault_Handler);
}

void MemManage_Handler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.MemManage_Handler);
}

void BusFault_Handler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.BusFault_Handler);
}

void UsageFault_Handler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.UsageFault_Handler);
}

void SVC_Handler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.SVC_Handler);
}

void DebugMon_Handler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.DebugMon_Handler);
}

void PendSV_Handler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.PendSV_Handler);
}

void SysTick_Handler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.SysTick_Handler);
}

void WAKEUP_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.WAKEUP_IRQHandler);
}

void I2C_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.I2C_IRQHandler);
}

void TIMER16_0_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.TIMER16_0_IRQHandler);
}

void TIMER16_1_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.TIMER16_1_IRQHandler);
}

void TIMER32_0_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.TIMER32_0_IRQHandler);
}

void TIMER32_1_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.TIMER32_1_IRQHandler);
}

void SSP_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.SSP_IRQHandler);
}

void UART_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.UART_IRQHandler);
}

void USB_IRQHandler(void)
{
	if (mode == 0)
		(*rom)->pUSBD->isr();
	else
		ASM_Goto(header->eps.USB_IRQHandler);
}

void USB_FIQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.USB_FIQHandler);
}

void ADC_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.ADC_IRQHandler);
}

void WDT_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.WDT_IRQHandler);
}

void BOD_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.BOD_IRQHandler);
}

void FMC_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.FMC_IRQHandler);
}

void PIOINT3_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.PIOINT3_IRQHandler);
}

void PIOINT2_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.PIOINT2_IRQHandler);
}

void PIOINT1_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.PIOINT1_IRQHandler);
}

void PIOINT0_IRQHandler(void)
{
	if (mode == 0)
	{
		while (1);
	}
	else
		ASM_Goto(header->eps.PIOINT0_IRQHandler);
}

ZERO_INIT uint8_t abDataIn[64];
ZERO_INIT uint8_t abData[64 - 8];
uint8_t bLen = 0;
uint8_t bProcessed = 0;
uint8_t bError = 0;

void GetInReport(uint8_t src[], uint32_t length)
{
	io_data_t *pdata = (io_data_t*)src;

	memset(src, 0x00, length);		

	if (bProcessed == 1)
	{
		bProcessed = 0;
		
		pdata->dev2pc.bRetStatus = bError;
		pdata->dev2pc.bLen = bLen;
		memcpy(pdata->dev2pc.abData, abData, bLen);
	}
	else
	{
		pdata->dev2pc.bRetStatus = 0xFF;
		pdata->dev2pc.bLen = 0;
	}
}

void SetOutReport(uint8_t dst[], uint32_t length)
{
	if (length > 64)
	{
		bError = 0xFD;
		bProcessed = 1;
	}
	else if (bProcessed != 0)
	{
	}
	else
	{
		bProcessed = 2;
		memcpy(abDataIn, dst, length);
	}
}

void frw_clean(void)
{
	int sect;

	NVIC_DisableIRQ(USB_IRQn);          // USB irq disable
	sect = iap_getlastsector();
	if (sect > 1)
	{
		// erase firmware:
		iap_prepare(1, sect);
		iap_erase(1, sect);
	}
	NVIC_EnableIRQ(USB_IRQn);          // USB irq disable
}

void frw_kill(void)
{
	int sect;

	frw_clean();

	NVIC_DisableIRQ(USB_IRQn);          // USB irq disable
	(*rom)->pUSBD->connect(FALSE);      // USB Disconnect
	// erase firmware:
	iap_prepare(0, 0);
	iap_erase(0, 0);
}

int is_usb(void)
{
	// usb
	return gio_get(0, 3);
}

ZERO_INIT uint8_t wbuffer[256];
const uint8_t key[32] = FRW_CRYPT_KEY;

int main(void)
{
	volatile int n;
	uint32_t cbc1 = 0, cbc2 = 0;

	{
		// IOCON
		LPC_SYSCON->SYSAHBCLKCTRL |= 0x00010000;
		// A. VBUS Fix:
		LPC_IOCON->PIO0_3 = 0x00000008;
		// B. Power On fix:
		LPC_IOCON->PIO3_3 = 0x00000000; // KEY ON / RES1
		LPC_IOCON->PIO2_4 = 0x00000000; // KEY OFF / RES2
		// C. LCD fix:
		LPC_IOCON->PIO3_2 = 0x00000000; // CS
		gio_out(3,2);
		gio_set0(3,2);
		LPC_IOCON->PIO3_1 = 0x00000000; // SCK
		gio_out(3,1);
		gio_set0(3,1);
		LPC_IOCON->PIO3_0 = 0x00000000; // MOSI
		gio_out(3,0);
		gio_set0(3,0);
		LPC_IOCON->PIO2_9 = 0x00000000; // PWR / RES3
		gio_out(2,9);
		gio_set0(2,9);
	}

	*sn = __sn;

	// mode zero init.
	mode = 0;

	// Wait:
	for (n = 0; n < 1000000; n++) { __NOP(); }
	// Init:
	SystemInit();
	
	if (*(uint32_t*)0x10000000 == 0xF387E238 && is_usb())
	{
		mode = 0;
		// no check module!
	}
	else if (header->tag == 0xAB6E37F4 &&
		header->size < (0x8000-0x1000-0x100) &&
		header->hw_ver == HARDWARE_VERSION &&
		header->crc32 == crc32(CRC32_FRW_START, header->data, header->size))
	{
		// module ok!
		mode = 1;
		ASM_GotoSP(header->eps.Reset_Handler, header->sp);
	}
	
	if (is_usb() == 0)
	{
		// no usb!
		while (1)
		{
			__WFI();
		}
	}

	HidDevInfo.idVendor = USB_VENDOR_ID;
	HidDevInfo.idProduct = USB_PROD_ID;
	HidDevInfo.bcdDevice = USB_DEVICE; 
	HidDevInfo.StrDescPtr = (uint32_t)&USB_StringDescriptor[0];
	HidDevInfo.InReportCount = 64;
	HidDevInfo.OutReportCount = 64;
	HidDevInfo.SampleInterval = 0x01;
	HidDevInfo.InReport = GetInReport;
	HidDevInfo.OutReport = SetOutReport;

	DeviceInfo.DevType = USB_DEVICE_CLASS_HUMAN_INTERFACE;
	DeviceInfo.DevDetailPtr = (uint32_t)&HidDevInfo;

	/* Enable Timer32_1, IOCON, and USB blocks (for USB ROM driver) */
	/* Already sets */
	//LPC_SYSCON->SYSAHBCLKCTRL |= (EN_TIMER32_1 | EN_IOCON | EN_USBREG);

#if 0
	/* Use pll and pin init function in rom */
	(*rom)->pUSBD->init_clk_pins();
#else
	// PLL already started
	LPC_IOCON->PIO0_3  &= ~0x07;              /* P0.3 VBUS */
	LPC_IOCON->PIO0_3  |=  0x01;              /* Select function USB_VBUS */
	LPC_IOCON->PIO0_6  &= ~0x07;              /* P0.6 SoftConnect */
	LPC_IOCON->PIO0_6  |=  0x01;              /* Selects function USB_CONNECT */
#endif

	/* insert a delay between clk init and usb init */
	for (n = 0; n < 75; n++) { __NOP(); }

	(*rom)->pUSBD->init(&DeviceInfo); /* USB Initialization */
	(*rom)->pUSBD->connect(TRUE);     /* USB Connect */

	// Инициализируем ключ шифрования:
	gost_init((uint32_t*)key);

	n = 0;
	// Основная логика загрузчика:
	while (1)
	{
		if (bProcessed == 2)
		{
			io_data_t *pdata = (io_data_t *)abDataIn;
			switch (pdata->pc2dev.bOperation)
			{
			case OP_FW_WRITE_RAM:
				{
					int l = pdata->pc2dev.bLen;
					
					if ((l < 8) ||
						(l & 0x07))
					{
						bError = 0x01;
						bLen = 0;
					}
					else
					{
						uint32_t cbc1l, cbc2l;

						// Вектор инициализации для CBC, запоминаем:
						cbc1l = ((uint32_t*)pdata->pc2dev.abData)[(l>>2)-2];
						cbc2l = ((uint32_t*)pdata->pc2dev.abData)[(l>>2)-1];
						
						if (n == 0)
						{
							// Первый блок, обнуляем буфер:
							memset(wbuffer, 0xFF, sizeof(wbuffer));
							gost_decrypt_cbc((uint32_t*)pdata->pc2dev.abData, l>>2, GOST_CBC_VECT_1, GOST_CBC_VECT_2);
						}
						else
						{
							gost_decrypt_cbc((uint32_t*)pdata->pc2dev.abData, l>>2, cbc1, cbc2);
						}

						// Сохраняем на следующий цикл:
						cbc1 = cbc1l;
						cbc2 = cbc2l;
						
						// Заполнение буфера:
						memcpy(&wbuffer[n&0xFF], pdata->pc2dev.abData, l);

						// Приращиваем длину данных:
						n += l;
						
						bError = 0;
						bLen = 0;
					}
				}
				break;
			case OP_FW_WRITE_BLOCK:
				{
					if (n == 0)
					{
						bError = 0x01;
						bLen = 0;
					}
					//else if (n & 0xFF)
					//{
					//	bError = 0x02;
					//	bLen = 0;
					//}
					else if (n > IMG_SIZE)
					{
						bError = 0x03;
						bLen = 0;
					}
					else if (pdata->pc2dev.bAddr || pdata->pc2dev.bLen)
					{
						bError = 0x04;
						bLen = 0;
					}
					else
					{
						int sect = iap_getlastsector();
						
						// Если блок загружен не полностью:
						if (n & 0xFF)
						{
							// Добиваем размер блока до полных 256 байт:
							n = (n & ~0xFF) + 0x100;
						}
						// Не полный блок в нормальной цепочке загрузки может 
						// встретиться только один раз: в случае загрузки образа 
						// размером не кратном 256 байтам. В данном случае таким 
						// добивочным будет последний блок данных.
						
						// Запись блока:
						iap_prepare(1, sect);
						iap_write256(wbuffer, (uint8_t*)(0x1000 + n - 0x100));
						// Обнуляем буфер:
						memset(wbuffer, 0xFF, sizeof(wbuffer));
						
						bError = 0x00;
						bLen = 0;
					}
				}
				break;
#if 1
			// Временная логика для чтения данных:
			case OP_FW_READ_BLOCK:
				{
					uint32_t dwAddr = pdata->pc2dev_read.dwAddr;
					bLen = pdata->pc2dev_read.bLen;
					if (bLen > 64-8) {
						bError = 0x01;
						bLen = 0;
					} else {
						// Обнуляем буфер:
						memcpy(abData, (void*)dwAddr, bLen);
						bError = 0x00;
					}					
				}
				break;
#endif
			case OP_FW_CLEAR:
				{
					uint8_t password_1[] = FW_CLEAR_VECTOR;
					uint8_t password_2[] = FW_KILL_VECTOR;
					if (pdata->pc2dev.bLen == sizeof(password_1) &&
						memcmp(pdata->pc2dev.abData, password_1, sizeof(password_1)) == 0)
					{
						frw_clean();
						n = 0;
						
						bError = 0x00;
						bLen = 0;
					}
					if (pdata->pc2dev.bLen == sizeof(password_2) &&
						memcmp(pdata->pc2dev.abData, password_2, sizeof(password_2)) == 0)
					{
						frw_kill();
						while (1);
					}
				}
				break;
			case OP_FW_GET_HW:
				{
					*(uint32_t*)abData = HARDWARE_VERSION;
					
					bError = 0x00;
					bLen = sizeof(uint32_t);
				}
				break;
//			case OP_FW_GET_HW:
//				{
//					*(uint32_t*)abData = HARDWARE_VERSION;
//					
//					bError = 0x00;
//					bLen = sizeof(uint32_t);
//				}
//				break;
			default:
				bError = 0xFE;
				bLen = 0;
				break;
			}
			bProcessed = 1;
		}
		__WFI();
	}

	return 0;
}
