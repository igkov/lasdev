/*
	main.c

	Основной проект дальномера.

	Сборка Release
		Сборка для загрузчика.
	Сборка Debug
		Автономная сборка.

	igorkov / 2013-2014 / igorkov.rus@gmail.com
 */
#include <string.h>
#include <math.h>
#include "lpc13xx.h"
#include "type.h"

#include "gio_lpc13xx.h"

#include "usb.h"
#include "usbdesc.h"
#include "usbrom.h"

#include "config.h"

#include "io.h"
#include "uart.h"
#include "i2c.h"
#include "iap.h"
#include "lsm303.h"
#include "event.h"
#include "leds.h"
#include "orientation.h"
#include "calibrate.h"

#include "spi.h"
#include "spi_eeprom.h"

#include "spi9.h"
#include "lcd.h"

#include "uni_t_proto.h"

#include "crc.h"

// Версия микропрограммы:
#define FIRMWARE_VERSION_H 0x02
#define FIRMWARE_VERSION_L 0x01

#define     EN_TIMER32_1    (1<<10)
#define     EN_IOCON        (1<<16)
#define     EN_USBREG       (1<<14)

USB_DEV_INFO DeviceInfo;
HID_DEVICE_INFO HidDevInfo;
ROM ** rom = (ROM **)0x1fff1ff8;

uint8_t abDataIn[64];
uint8_t abData[64 - 8];
uint8_t bLen = 0;
uint8_t bProcessed = 0;
uint8_t bError = 0;

#define SERIAL_NUMBER (*(uint32_t*)0x10000008)

extern void SoftwareReset(void);
extern uint32_t GetHwVersion(void);

extern uint32_t SystemFrequency;

#if defined( BLUETOOTH )
/* Структура посылки: */

typedef struct
{
	uint8_t type;
	uint8_t size;
	uint8_t crc8;
	uint8_t res;
	uint8_t data[];
} proto_t, *pproto_t;

#define COMM_MAX_SIZE 128

#define COMM_E_OK        0x00
#define COMM_E_SIZE      0x01
#define COMM_E_OFFSET    0x02
#define COMM_E_CRC       0x03
#define COMM_E_BAD_TYPE  0x04
#define COMM_E_HW_ERR    0x05
#define COMM_E_PASS      0x06
#define COMM_E_PARAM     0x07

uint8_t comm[COMM_MAX_SIZE];

#endif // BLUETOOTH

/*
 *  Get HID Input Report -> InReport
 */
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
		
		led_red(0);
	}
	else
	{
		pdata->dev2pc.bRetStatus = 0xFF;
		pdata->dev2pc.bLen = 0;
	}
}

/*
 *  Set HID Output Report <- OutReport
 */
void SetOutReport(uint8_t dst[], uint32_t length)
{
	led_red(1);
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

#define MODE_USB      0x00
#define MODE_NATIVE   0x01
#define MODE_CALIBRATION   0x02
#define MODE_BT       0x03

config_t config;

uint8_t configure = 0;
uint8_t mode = MODE_USB;

// Тест асинхронного события:
void led_blink(void)
{
	static uint8_t led_state = 0;
	led_state = led_state ? 0 : 1;
	led_red(led_state);
	
	event_set(led_blink, 500);
}

void frw_dispatch(int p)
{
	// Disable peripheral:
	(*rom)->pUSBD->connect(FALSE);      // USB Disconnect
	SysTick->CTRL  = 0;                 // Systick irq disable
	NVIC_DisableIRQ(ADC_IRQn);          // ADC irq disable
	NVIC_DisableIRQ(TIMER_32_0_IRQn);   // Timer0 irq disable
	NVIC_DisableIRQ(UART_IRQn);         // UART irq disable
	NVIC_DisableIRQ(USB_IRQn);          // USB irq disable
					
	if (p == 0)
	{
		int sect;
		
		sect = iap_getlastsector();
		if (sect > 1)
		{
	#if defined( RELEASE )
			// В случае RELEASE-сборки микропрограммы не стираем 0ой сектор!
			// Там загрузчик!
			
			// erase firmware:
			iap_prepare(1, sect);
			iap_erase(1, sect);
	#else
			// erase firmware:
			iap_prepare(0, sect);
			iap_erase(0, sect);
	#endif					
		}
	}
	else if (p == 1)
	{
		int n;
		uint32_t *data = (uint32_t*)0x10000000;
		
		// Кодовый верктор для входа в загрузчик:
		*data = 0xF387E238; // Bootloader flag!
		
		// Задержка перед программной перезагрузкой:
		for (n=0; n<1000000; n++) { __NOP(); }
		
		SoftwareReset();
	}

	// Данный код не исполняется, т.к. возврат из erase 
	// происходит, когда уже все стерто.
	while (1);
}

int get_imu(pvector3df_t acc_cal, pvector3df_t mag_cal, float *tilt, float *azimuth)
{
	int ret;
	float g;

	if (configure)
	{
		// Проверяем, что устройстве не в движении:
		g = v3f_mod(acc_cal);
		if (g < 1.05f && g > 0.95f)
		{
			// ok
		}
		else
		{
			// move detected!
			return 2;
		}
#if 0 // в целях отладки закрываем
		m = v3f_mod(mag_cal);
		if (g < 1.05f && m > 0.95f)
		{
			// ok
		}
		else
		{
			// magnetic detected!
			return 1;
		}
#endif
		// Нормируем вектора:
		v3f_norm(acc_cal);
		v3f_norm(mag_cal);
		
		// Вычисление углов (основная функция 3D-ядра):
		ret = calc_angles(acc_cal, mag_cal, tilt, azimuth);
		
		// Вносим поправки в значения
		*azimuth += config.correct_mag;
		*tilt    += config.correct_tilt;
		
		return ret;
	}
	else
	{
		// no configuration!
		return 2;
	}
}

int caln = -1;
void event_caln_clr(void)
{
	caln = -1;
}


void lcd_init(void)
{
#if 1
	// Включаем питание LCD:
	gio_set1(2,9); // lcd power on
	spi9_unselect();
	
	delay_ms(50);
	gio_set0(2,9); // lcd power off
	spi9_select();
	delay_ms(700);
	gio_set1(2,9); // lcd power on
	spi9_unselect();
	delay_ms(20);

	// Заполнение значений, в начале прочерками:
	lcd_arr(lcd_line_1_nop, 1);
	lcd_arr(lcd_line_2_nop, 1);
	lcd_arr(lcd_line_3_nop, 1);
	
	// Посылка кадра на LCD:
	send_null_frame(); delay_ms(1);
	send_null_frame(); delay_ms(1);
	send_null_frame(); delay_ms(1);
	send_null_frame(); delay_ms(10);
	send_frame(); delay_ms(1);
	send_frame(); delay_ms(1);
	send_frame(); delay_ms(1);
	send_frame(); delay_ms(10);
#endif
}

uint8_t key_on_flag;
uint8_t key_c_flag;

void PIOINT2_IRQHandler(void)
{
  	if ( LPC_GPIO2->MIS & (0x1<<4) )	// pio 2.4
  	{
		key_c_flag = 1;
		LPC_GPIO2->IC |= (0x1<<4);
  	}
}

void PIOINT3_IRQHandler(void)
{
  	if ( LPC_GPIO3->MIS & (0x1<<3) )	// pio 3.3
  	{
		key_on_flag = 1;
		LPC_GPIO3->IC |= (0x1<<3);
  	}
}

int get_info(pdevice_info_t pdev_info)
{
	int i;
	pdev_info->frw_version_h = FIRMWARE_VERSION_H;
	pdev_info->frw_version_l = FIRMWARE_VERSION_L;
	pdev_info->configure = configure;
	pdev_info->error = 0;
	pdev_info->serial_number = SERIAL_NUMBER;
	pdev_info->memory_size = config.memsize * 8192;
#if defined( RELEASE )
	pdev_info->hrw_version = ((pimg_bin_t)0x00001000)->hw_ver;
#else
	pdev_info->hrw_version = GetHwVersion();
#endif
	if (configure)
	{
		// Ищем последнюю запись в таблице лога:
		for (i=0; i<(pdev_info->memory_size-MEM_LOG_OFFSET)/sizeof(log_unit_t); i++)
		{
			log_unit_t unit;
			//uint8_t crc;
			bError = ee_read(MEM_LOG_OFFSET+i*sizeof(log_unit_t), (uint8_t*)&unit, sizeof(log_unit_t));
			if (bError)
			{
				return bError;
			}
			if (unit.crc8 == 0xFF &&
				unit.type == 0xFF)
			{
				// null row, end!
				break;
			}
			//crc = unit.crc8;
			//unit.crc8 = 0xFF;
			//if (crc != crc8((uint8_t*)&unit, sizeof(log_unit_t)))
			//	break;
		}
		pdev_info->log_len = i;
	}
	else
	{
		pdev_info->log_len = 0;
	}
	return 0;
}

vector3d_t cal_values_a[CAL_MAX_K];
vector3d_t cal_values_m[CAL_MAX_K];

int main(void)
{
	int ret;
	volatile int n;
	uint16_t crc;

#if !defined( RELEASE )
	for (n = 0; n < 1000000; n++) { __NOP(); }
#endif

#if !defined( RELEASE )
	SERIAL_NUMBER = 0xAABBCCDD;
#endif

	// System Logic Init:
	SystemInit();

	// Program SPI9, for LCD:
	spi9_init();

	// Event System Init:
	event_init();

	// UART initialize:
	uart_init(115200, SystemFrequency);

	// I2C initialize Hardware:
	i2c_init();

#if 1
	// Init sensor:
	lsm303_init( 0,    1,        1       );
	//           2g,   1.3Gauss, Temp En
	//lsm303_init( 0,    4,        1       );
	//           2g,   4.0Gauss, Temp En
	// Note: С определенного момента была смена чувствительности. 
	// Используется максимальная для данной модели дальномера.
	// Todo: Уменьшить частоты дискредитации данных.
#endif
	
	// LEDs init
	leds_init();

	// SPI init	
	spi_init();
	
	// Laser init
	uni_t_init();

	// C-button init:
	gio_in(2,4);
	LPC_IOCON->PIO2_4 = 1<<3; // pull-down

#if 1
	LPC_GPIO2->IS  &= ~(0x1<<4); // 2.4
	LPC_GPIO2->IBE &= ~(0x1<<4);
	LPC_GPIO2->IEV &= ~(0x1<<4);
	LPC_GPIO2->IE  |=  (0x1<<4);
	LPC_GPIO3->IS  &= ~(0x1<<3); // 3.3
	LPC_GPIO3->IBE &= ~(0x1<<3);
	LPC_GPIO3->IEV &= ~(0x1<<3);
	LPC_GPIO3->IE  |=  (0x1<<3);
	
	NVIC_EnableIRQ(EINT2_IRQn);
	NVIC_EnableIRQ(EINT3_IRQn);

	key_on_flag = 0;
	key_c_flag = 0;
#endif

	// Read configuration
	ret = ee_read(0, (uint8_t*)&config, sizeof(config_t));
	if (ret) bError = 1;
	crc = config.crc16;
	config.crc16 = 0xFFFF;
	if (config.tag == 0xEA && 
		config.version == 0x01 &&
		crc == crc16((uint8_t*)&config, sizeof(config_t)) &&
		config.sn == SERIAL_NUMBER)
	{
		configure = 1;
	}
	
	// По сигналу VBUS определяем, какой режим работы требуется:
	if (gio_get(0, 3))
	{
		mode = MODE_USB;
	}
	else
	{
		mode = MODE_NATIVE;
	}

	if (mode == MODE_NATIVE)
	{
		//int dist = -1;
		float tilt;
		float azimuth;
		int accn = -1;
		
		// Вектора с сырыми данными с датчиков:
		vector3d_t acc_first;
		// Вектора с сырыми данными с датчиков:
		vector3df_t acc_accum;
		vector3df_t mag_accum;
		// Вектора с сырыми данными с датчиков:
		vector3d_t acc;
		vector3d_t mag;
		
		lcd_init();
		
		if (configure == 0)
		{
			// Нет записанной конфигуркции:
			while (1)
			{
				// Выводим значок ключика, сигнализирующий о том, 
				// что нужна калибровка и настройка:
				accn = accn ? 0 : 1;
				lcd_arr(lcd_setup, accn);
				
				// Посылка кадра в экран (должна повторяться постоянно):
				send_frame(); delay_ms(1);
				send_frame(); delay_ms(1);
				send_frame(); delay_ms(1);
				send_frame(); delay_ms(1);
				
				// Задержка цикла:
				delay_ms(700);
			}
		}
		else
		{
			// Вывод служебных символов на экран
			//lcd_arr(lcd_arr_batt, 1);
			lcd_arr(lcd_arr_las, 1);
			lcd_arr(lcd_unit, 1);
			
			//
			//lcd_arr(lcd_setup, 1);
			//lcd_arr(lcd_cal, 1);
			//lcd_arr(lcd_of_32, 1);
			
			// Заполнение значений, в начале прочерками:
			lcd_arr(lcd_line_1_nop, 1);
			lcd_arr(lcd_line_2_nop, 1);
			lcd_arr(lcd_line_3_nop, 1);
			
			send_frame();
			delay_ms(1);
			send_frame();
			delay_ms(1);
			send_frame();
			delay_ms(1);
			delay_ms(10);
		}
		
		/*
			// Логика работы клавиатуры:
			
			uint8_t new_event;
			
			#define KEY_NOP   0x00
			#define KEY_OK    0x01
			#define KEY_C     0x02
			#define KEY_FUNC  0x03
			
			key = check_key();
			if (key == KEY_NOP)
			{
				// ничего не поменялось
			}
			else if (key == KEY_OK)
			{
				// ok key
			}
			else if (key == KEY_C)
			{
				// 
			}
			else if (key == KEY_FUNC)
			{
				//
			}
		 */
		
		#define IMU_ACCUMULATE_N    6
		#define IMU_ACCUMULATE_C    50
		#define IMU_ACCUMULATE_DIFF	0.03f
		
		n = 0;
		
		// Автономный режим.
		// В настоящий момент цикл с выводом текущих измерений на экран.
		while (1)
		{
			// Алгоритм получения IMU:
			// 1. Получает вектор в 1ом цикле.
			// 2. Получает следующий вектор измерения.
			// 3. Вычитает из 1ого вектора кадлый следующий.
			// 4. Если получает отклонение в пределах 5% (??), тогда накапливает новый вектор.
			// 5. Если отклонение превышено, тогда последний полученный ставиться начальным и цикл начинается по новой.
			// 6. Если число векторов в накоплении меньше 5-10, тогда переходит на пункт 2.
			// 7. Если накоплено достаточно векторов, тогда считается среднее.
			
			// Получаем вектор ускорения:
			ret  = lsm303_get_acc(&acc);
			if (ret)
			{
				lcd_arr(lcd_line_2_err, 1);
				accn = -1;
			}
			// Получаем вектор магнитного поля:
			ret = lsm303_get_mag(&mag);
			if (ret)
			{
				lcd_arr(lcd_line_3_err, 1);
				accn = -1;
			}
			// Логика накопления измерений:
			if (accn == 0)
			{
				acc_first = acc;
				acc_accum.x = acc.x;
				acc_accum.y = acc.y;
				acc_accum.z = acc.z;
				mag_accum.x = mag.x;
				mag_accum.y = mag.y;
				mag_accum.z = mag.z;
			}
			else if (accn > 0 &&
					accn < IMU_ACCUMULATE_N)
			{
				// Накапливаем:
				//v3_add(&acc_accum, &acc_temp);
				acc_accum.x += acc.x;
				acc_accum.y += acc.y;
				acc_accum.z += acc.z;
				//v3_add(&mag_accum, &mag);
				mag_accum.x += mag.x;
				mag_accum.y += mag.y;
				mag_accum.z += mag.z;
#if 0
				// Дельта векторов:
				v3_sub(&acc, &acc_first);
				// Проверяем разность:
				if ((float)v3_mod(&acc) / (float)v3_mod(&acc_first) > IMU_ACCUMULATE_DIFF)
				{
					// Не накопили!
					accn = -1;
				}
#else
				// 2014.11.14: Более строгие проверки.
				// Дельта векторов:
				v3_sub(&acc, &acc_first);
				// Проверяем совпадение:
				if (ABS((float)acc.x/(float)acc_first.x) > IMU_ACCUMULATE_DIFF ||
					ABS((float)acc.y/(float)acc_first.y) > IMU_ACCUMULATE_DIFF ||
					ABS((float)acc.z/(float)acc_first.z) > IMU_ACCUMULATE_DIFF)
				{
					// Не накопили!
					accn = -1;
				}
#endif
			}
			// Добавлено измерение:
			accn++;
			// Проверяем, не накопили ли нужное количество:
			if (accn == IMU_ACCUMULATE_N)
			{
				// Калиброванне значения:
				vector3df_t acc_cal;
				vector3df_t mag_cal;
				// Считаем среднее у накопленного:
				acc_accum.x /= IMU_ACCUMULATE_N;
				acc_accum.y /= IMU_ACCUMULATE_N;
				acc_accum.z /= IMU_ACCUMULATE_N;
				mag_accum.x /= IMU_ACCUMULATE_N;
				mag_accum.y /= IMU_ACCUMULATE_N;
				mag_accum.z /= IMU_ACCUMULATE_N;
				// Копируем в целочисленный вектор:
				acc.x = acc_accum.x;
				acc.y = acc_accum.y;
				acc.z = acc_accum.z;
				mag.x = mag_accum.x;
				mag.y = mag_accum.y;
				mag.z = mag_accum.z;
				
				if (mode == MODE_NATIVE)
				{
					// Накладываем калибровку:
					calibration_proc(&acc_cal, &mag_cal, &acc, &mag, &config);
					// Вычисляем углы:
					ret = get_imu(&acc_cal, &mag_cal, &tilt, &azimuth);
					if (ret == 1)
					{
						// Азимут не определен:
						put_clr(2);
						lcd_arr(lcd_line_3_nop, 1);
						// Угол определен:
						put_tilt((int)(tilt*10));
					}
					else if (ret == 2)
					{
						// Азимут не определен:
						put_clr(2);
						lcd_arr(lcd_line_3_nop, 1);
						// Угол не определен:
						put_clr(1);
						lcd_arr(lcd_line_2_nop, 1);
					}
					else
					{
						// Азимут определен:
						put_azimuth((int)(azimuth*10));
						// Угол определен:
						put_clr(1);
						put_tilt((int)(tilt*10));
					}
				}
				else if (mode == MODE_CALIBRATION)
				{
					if (key_on_flag == 1)
					{
						if (caln == 0)
						{
							// Сброс контекста.
							// NOP
						}
						
						cal_values_a[caln] = acc;
						cal_values_m[caln] = mag;
						
						// След. значение:
						caln++;
						
						if (caln == 32)
						{
							// Калибровка:
						   	ret = calibration_calc_int(cal_values_a, cal_values_m, caln, &config.acc_cal, &config.mag_cal);
							// 
							if (ret == 0)
							{
								// Обновление конфигурации:
								config.crc16 = 0xFFFF;
								config.crc16 = crc16((uint8_t*)&config, sizeof(config_t));
								
								for (n=0; n<sizeof(config_t); n+=32)
								{
									// Запись конфигурации:
									ret = ee_write(n, (uint8_t*)&(((uint8_t*)&config)[n]), 32);
									if (ret)
										goto calibration_error;
								}
								
								// Вывод сообщения
								put_clr(0);
								lcd_arr(lcd_end, 1);
							}
							else
							{
calibration_error:
								put_clr(0);
								lcd_arr(lcd_line_1_err, 1);
							}
							
							caln = -1;
							mode = MODE_NATIVE;
							
							// Задержка:
							delay_ms(500);
						}
						else
						{
							put_clr(2);
							put_cnt(caln+1);
							
							// Задержка:
							delay_ms(300);
						}
						
						// Обнуление
						key_on_flag = 0;
					}
				}
				else
				{
					// unknown
				}
			}
			// При достижении опред. кол-ва, сбрасываемся:
			if (accn == IMU_ACCUMULATE_C)
				accn = 0;

#if 0			
			// Получение дистанции:
			ret = get_dist_check();
			switch (ret)
			{
			case 0:
				// Есть новые данные, получаем:
				ret = get_dist_end(&dist);
				if (ret != 0)
				{
					// Ошибка в поле дистанции:
					lcd_arr(lcd_line_1_err, 1);
				}
				else if (dist == 0)
				{
					lcd_arr(lcd_line_1_nop, 1);
				}
				else
				{
					// Корректировочное значение дистанции:
					dist += config.correct_dist;
					// Выводим текущее расстояние:
					put_dist(dist);
				}
				break;
			case 1:
				// Идет процесс измерения.
				break;
			case 2:
				// Нет данных, запрашиваем новые:
				start_receive();
				break;
			case 3:
			case 4:
				//lcd_arr(lcd_line1_err, 1);
				// Таймаут, не считаем за ошибку.
				start_receive();
				break;
			}
#else
			if (mode == MODE_NATIVE)
			{
				extern int Dist;
				put_clr(0);
				if (Dist == -1)
				{
					lcd_arr(lcd_line_1_nop, 1);
				}
				else if (Dist == -2)
				{
					lcd_arr(lcd_line_1_err, 1);
				}
				else
				{
					// Выводим текущее расстояние:
					put_dist(Dist);
				}
			}
#endif
			// Проверяем нажатие C-клавиши.
			// Обнуляет показания и контролирует переход в режим калибровки.
			if (key_c_flag == 1 &&
				mode == MODE_NATIVE)
			{
				// Начинаем проход в режим калибровки.
				if (caln == -1)
				{
					caln = 1;
					event_set(event_caln_clr, 4000); // 5 нажатий за 4 секунды
				}
				else
				{
					caln++;
					// Если набрали нужное количество нажатий:
					if (caln == 5)
					{
						event_unset(event_caln_clr);
						// Переходим в режим калибровки.
						caln = 0;
						mode = MODE_CALIBRATION;
						
						// Вывод информации на экран:
						
						// line 1 - of 32
						put_clr(1);
						lcd_arr(lcd_of_32, 1);
						
						// line 2 - 1
						put_clr(2);
						put_cnt(caln+1);
					}
				}
				
				// Обнуляем дистанцию:
				Dist = -1;
				
				// Выводим подсказку CAL:
				
				// line 0 - CAL
				put_clr(0);
				lcd_arr(lcd_cal, 1);
				send_frame();
				send_frame();
				send_frame();
				delay_ms(300);

				key_c_flag = 0;
				key_on_flag = 0;
			}
			
			// Посылка кадра в экран (должна повторяться постоянно):
			send_frame();
			// Задержка цикла:
			delay_ms(10);
		}
	}
#if defined( BLUETOOTH )
	else if (mode == MODE_BT)
	{
		int offset;
		uint8_t b;
		pproto_t hdr;
bt_mode:
		// Отключаем прерывания по UART.
		// В режиме BT работаем с портом синхронно.
		uart_irq(0);
		// Протокол BT (готовим о начале):
		uart_command("lasdev in BT mode!\r\n", 20);
		// Переменные протокола:
		offset = -1;
		hdr = (pproto_t)comm;
		// Основной цикл:
		while (1)
		{
			b = uart_getchar();
			// Эхо для теста:
			//uart_putchar(b); // for dbg
			// Работа протокола:
			switch(b)
			{
			case ':':
				// сброс протокола.
				memset(comm, 0, COMM_MAX_SIZE);
				offset = 0;
			case '\n':
				break;
			case '\r':
				if (offset == -1)
					break;
				if (offset & 0x01)
				{
					// error, offset
					b = COMM_E_OFFSET;
					goto comm_error;
				}
				// делим на 2, чтобы получить реальную длину команды:
				offset /= 2;
				// проверка длины:
				if (hdr->size != offset - sizeof(proto_t))
				{
					// error, size
					b = COMM_E_SIZE;
					goto comm_error;
				}
				// проверка CRC:
				if (hdr->size != 0 &&
					hdr->crc8 != crc8(hdr->data, hdr->size))
				{
					// error, size
					b = COMM_E_CRC;
					goto comm_error;
				}
				// переход на обработку команды
				goto comm_work;
			default:
				if (offset == -1)
					break;
				// char2hex
				if (b <= '9' && b >= '0')
				{
					b -= '0';
				}	
				else if (b <= 'f' && b >= 'a')
				{
					b -= 'a' - 10;
				}
				else if (b <= 'F' && b >= 'A')
				{
					b -= 'A' - 10;
				}
				else break; // ignore
				
				if (offset/2 == COMM_MAX_SIZE)
				{
					// error, size
					b = COMM_E_SIZE;
					goto comm_error;
				}
				else
				{
					comm[offset/2] |= b << (offset & 0x01 ? 0 : 4);
					offset++;
				}
			}
			continue;
comm_work:
			// Обработка команды:
			switch (hdr->type)
			{
			case 0xFF:
				// End Bluetooth session:
				goto comm_end;
			case 0x00:
				// Test command:
				{
					char ack_data[] = "Hello lasdev!\r\n";
					#define ACK_SIZE 15
					hdr->type = COMM_E_OK;
					hdr->size = ACK_SIZE;
					memcpy(hdr->data, ack_data, ACK_SIZE);
					hdr->crc8 = crc8((uint8_t*)ack_data, ACK_SIZE);
					hdr->res  = 0;
					goto comm_ack;
				}
			case 0x01:
				// Get time:
				{
					uint32_t clk_value = get_ms_timer();
					hdr->type = COMM_E_OK;
					hdr->size = 4;
					hdr->res  = 0;
					//cli();
					hdr->data[0] = (uint8_t)(clk_value>>24);
					hdr->data[1] = (uint8_t)(clk_value>>16);
					hdr->data[2] = (uint8_t)(clk_value>>8);
					hdr->data[3] = (uint8_t)(clk_value>>0);
					//sei();
					hdr->crc8 = crc8(hdr->data, hdr->size);
					goto comm_ack;
				}
			case 0x05:
				// Read config/info...
				{
					pdevice_info_t pdev_info = (pdevice_info_t)hdr->data;
					hdr->type = COMM_E_OK;
					hdr->size = sizeof(device_info_t);
					hdr->res  = 0;
					memset(hdr->data, 0x00, sizeof(device_info_t));
					get_info(pdev_info);
					hdr->crc8 = crc8(hdr->data, hdr->size);
					goto comm_ack;
				}
			case 0x08:
				// Read Sensor:
				{
					vector3d_t acc;
					vector3d_t mag;
					b  = lsm303_get_acc(&acc);
					b |= lsm303_get_mag(&mag);
					if (b)
					{
						b = COMM_E_HW_ERR;
						goto comm_error;
					}
					hdr->type = COMM_E_OK;
					hdr->size = sizeof(vector3d_t)*2; // 12 bytes
					hdr->res  = 0;
					((pvector3d_t)hdr->data)[0] = acc;
					((pvector3d_t)hdr->data)[1] = mag;
					hdr->crc8 = crc8(hdr->data, hdr->size);
					goto comm_ack;
				}
			case 0x09:
				// Read EEPROM:
				{
					uint8_t size;
					uint16_t addr;
					if (hdr->size == 2)
					{
						addr = hdr->data[0];
						size = hdr->data[1];
					}
					else if (hdr->size == 3)
					{
						addr = *(uint16_t*)hdr->data;
						size = hdr->data[2];
					}
					else
					{
						b = COMM_E_SIZE;
						goto comm_error;
					}
					hdr->type = COMM_E_OK;
					hdr->size = size;
					hdr->res  = 0;
					b = ee_read(addr, hdr->data, hdr->size); // ret code!
					if (b)
					{
						b = COMM_E_HW_ERR;
						goto comm_error;
					}
					hdr->crc8 = crc8(hdr->data, hdr->size);
					goto comm_ack;
				}
			case 0x02: // Set time:
			case 0x03: // Clear FLASH memory (clear log memory):
			case 0x04: // Read FLASH memory:
			case 0x06: // Write config...
			case 0x07: // Get voltage (test command):
			case 0x0A: // Write EEPROM:
			case 0x0C: // Read log unit:
			default:
				b = COMM_E_BAD_TYPE;
				goto comm_error;
			}
			goto comm_next;
comm_error:
			// Ошибка (код в b):
			hdr->type = b;
			hdr->size = 0;
			hdr->crc8 = 0xFF;
			hdr->res  = 0;
comm_ack:
			uart_putchar(':');
			for (offset = 0; offset < (hdr->size + sizeof(proto_t)); offset++)
			{
				b = comm[offset];
				if ((b & 0xF0) < 0xA0)
				{
					uart_putchar('0' + (b>>4));
				}
				else
				{
					uart_putchar('A' - 10 + (b>>4));
				}
				if ((b & 0x0F) < 0x0A)
				{
					uart_putchar('0' + (b&0x0F));
				}
				else
				{
					uart_putchar('A' - 10 + (b&0x0F));
				}
			}
			uart_putchar('\r');
			uart_putchar('\n'); // for dbg
			offset = -1;
comm_next:
			// Переход на след. команду:
			continue;
		}
comm_end:
		// Завершен протокол BT, возвращаемся в режим USB:
		mode = MODE_USB;
	}
#endif
	else if (mode == MODE_USB)
	{
		// Продолжаем выполнять код ниже.
		// NOP
	}
	else
	{
		// Неизвестный режим работы. Переходим в режим загрузчика.
		frw_dispatch(1);
	}

	lcd_init();

	// Настройка встроенного USB-стека:
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

	// Инициализация USB, подключение к шине:
	(*rom)->pUSBD->init(&DeviceInfo); /* USB Initialization */
	(*rom)->pUSBD->connect(TRUE);     /* USB Connect */

	while (1)
	{
#if defined( BLUETOOTH )
		if (mode == MODE_BT)
		{
			(*rom)->pUSBD->connect(FALSE);     /* USB Disconnect */
			goto bt_mode;
		}
#endif
		if (bProcessed == 2)
		{
			io_data_t *pdata = (io_data_t *)abDataIn;
			switch (pdata->pc2dev.bOperation)
			{
			case OP_GET_INFO:
				{
					pdevice_info_t pdev_info = (pdevice_info_t)abData;
					memset(abData, 0x00, sizeof(device_info_t));
					if (get_info(pdev_info))
						goto point_processed;
				}
				bLen = sizeof(device_info_t);
				bError = 0;
				break;
			case OP_GET_LSM_ACC:
				bLen = sizeof(vector3d_t);
				bError = lsm303_get_acc((pvector3d_t)abData);
				break;
			case OP_GET_LSM_MAG:
				bLen = sizeof(vector3d_t);
				bError  = lsm303_get_mag((pvector3d_t)abData);
				break;
			case OP_GET_LSM_DATA:
				bLen = sizeof(vector3d_t) * 2;
				bError   = lsm303_get_acc(&(((pvector3d_t)abData)[0]));
				bError  |= lsm303_get_mag(&(((pvector3d_t)abData)[1]));
				break;
			case OP_GET_LSM_TEMP:
				bLen = sizeof(uint16_t);
				bError   = lsm303_get_temp((uint16_t*)abData);
				break;
			case OP_EEREAD:
				bLen = pdata->pc2dev_ee.bLen;
				bError = ee_read(pdata->pc2dev_ee.wAddr, abData, bLen);
				break;
			case OP_EEWRITE:
				bLen = 0;
				ee_write_en();
				bError = ee_write(pdata->pc2dev_ee.wAddr, pdata->pc2dev_ee.abData, pdata->pc2dev_ee.bLen);
				ee_write_dis();
				break;
			case OP_GET_IMU:
				if (configure)
				{
					vector3d_t acc;
					vector3d_t mag;
					
					// Калибровочные значния:
					vector3df_t acc_cal = { 0.0f, 0.0f, 0.0f };
					vector3df_t mag_cal = { 0.0f, 0.0f, 0.0f };
					
					bError  = lsm303_get_acc(&acc);
					if (bError) 
					{
						goto _op_get_imu_end;
					}
					
					bError  = lsm303_get_mag(&mag);
					if (bError) 
					{
						goto _op_get_imu_end;
					}
					
					// Накладываем калибровку:
					calibration_proc(&acc_cal, &mag_cal, &acc, &mag, &config);
					
#if 0 // old
					// Проверяем, что устройстве не в движении:
					g = v3f_mod(&acc_cal);
					if (g < 1.05f && g > 0.95f)
					{
						((pimu_data_t)abData)->status = 0; // ok
					}
					else
					{
						((pimu_data_t)abData)->status = 1; // move detected!
					}
					
					// Нормируем вектора:
					v3f_norm(&acc_cal);
					v3f_norm(&mag_cal);
					
					// Вычисление углов (основная функция 3D-ядра):
					bError = calc_angles(&acc_cal, &mag_cal, &tilt, &azimuth);
					
					// Вносим поправки в значения
					azimuth += config.correct_mag;
					tilt    += config.correct_tilt;
					
					if (((pimu_data_t)abData)->status == 0)
					{
						if (bError)
						{
							((pimu_data_t)abData)->status = 2;
						}
					}
					
					((pimu_data_t)abData)->azimuth = azimuth;
					((pimu_data_t)abData)->tilt    = tilt;
					bError = 0;
#else
					bError = get_imu(&acc_cal, &mag_cal, &((pimu_data_t)abData)->tilt, &((pimu_data_t)abData)->azimuth);
					if (bError == 0)
					{
						((pimu_data_t)abData)->status = 0; // ok
					}
					else if (bError == 1)
					{
						((pimu_data_t)abData)->status = 2; // ok
					}
					else if (bError == 2)
					{
						((pimu_data_t)abData)->status = 1;
					}
					else
						goto _op_get_imu_end;
#endif
					bError = 0;
					bLen = sizeof(imu_data_t);
				}
				else
				{
					bError = 1;
_op_get_imu_end:
					bLen = 0;
				}
				break;
			case OP_GET_DIST:
				{
#if 0
					// Получение дистации из прибора:
					get_dist_start();
					bError = get_dist_end((int*)abData);
#else
					extern int Dist;
					*(int*)abData = Dist;
					if (Dist > 0)
						bError = 0;
					else 
						bError = Dist;
#endif
					if (configure)
					{
						*(int*)abData += config.correct_dist;
					}
					bLen = sizeof(int);
				}
				break;
#if 0
			case OP_GET_DIST_START:
				{
					// Получение дистации из прибора:
					bError = get_dist_start();
					bLen = 0;
				}
				break;
			case OP_GET_DIST_END:
				{
					bError = get_dist_check();
					if (bError == 0)
					{
						bError = get_dist_end((int*)abData);
						if (configure)
						{
							*(int*)abData += config.correct_dist;
						}
						bLen = sizeof(int);
					}
					else
					{
						bLen = 0;
					}
				}
				break;
#endif
#if 1 //!defined( RELEASE )
			case OP_I2C_SEND:
				if (pdata->pc2dev.bLen==2)
				{
					abData[0] = i2c(pdata->pc2dev.abData[0], pdata->pc2dev.abData[1]);
					bError = 0;
					bLen = 1;
				}
				else
				{
					bError = 0xFE;
					bLen = 0;
				}
				break;
			case OP_LAS_POWER_ON:
				power_on();
				bError = 0;
				bLen = 0;
				break;
			case OP_LAS_POWER_OFF:
				power_down();
				bError = 0;
				bLen = 0;
				break;
			case OP_LAS_ENABLE:
				laser_on();
				bError = 0;
				bLen = 0;
				break;
			case OP_LAS_LCD_SEND:
				{
					// only for debug
					set_frame(pdata->pc2dev.abData, pdata->pc2dev.bLen);
					// sending frame
					spi9_select();
					for (n = 0; n < (32+9); n++)
					{
						spi9_send(get_lcd_word(n));
					}
					spi9_unselect();
					delay_ms(10);
				}
				break;
			case OP_LAS_LCD_SEND_LOW:
				{
					// sending frame
					spi9_select();
					spi9_send(0x01FC);
					spi9_send(0x0130);
					spi9_send(0x0180);
					spi9_send(0x01AE);
					spi9_send(0x01D0);
					spi9_send(0x01E0);
					spi9_send(0x00F0);
					spi9_send(0x001A);
					spi9_send(0x0000);
					spi9_unselect();
					delay_ms(10);
				}
				break;
#endif
			case OP_FW_CLEAR:
				{
					uint8_t password[] = FW_CLEAR_VECTOR;
					if (pdata->pc2dev.bLen == sizeof(password) &&
						memcmp(pdata->pc2dev.abData, password, sizeof(password)) == 0)
					{
						frw_dispatch(0);
					}
				}
			case OP_FW_SWITCH:
				{
					// Переключение в режим загрузчика:
					frw_dispatch(1);
				}
			default:
				bError = 0xFE;
				bLen = 0;
				break;
			}
point_processed:
			bProcessed = 1;
		}
		__WFI();
	}
}

void USB_IRQHandler()
{
	(*rom)->pUSBD->isr();
}
