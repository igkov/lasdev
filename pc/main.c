/**
	Main utility for Laser Distance Meter
	
	[+] Графический режим утилиты (отображение компаса).
	[+] Концепция серийных номеров устройства. Серийный номер прописывается в модуле Loader 
	    по зарезервированному адресу. Оттуда его читает микропрограмма. По всем серийным номерам 
	    ведется база данных.
	[+] Логика обновления микропрограммы: генерация, стирание, заливка.
	[+] Функционал генерации образа загрузчика с установкой CRP и Serial Number.
	[+] Логика работы с дальномером.
	[+] Поддержка различных выходных форматов.
	[-] UART-мост, чтобы взаимодействовать с дальномером на прямую.
	(?) Не всегда происходит получение ответа от дальномера на команду GetDistantion!
	(!) Новые калибровочные таблицы.
	[+] Тестовое управление LCD-экраном.
	[ ] Проверка дрейфа при температурных колебаниях, особенно для магнетометра.
	[+] Косметические исправления на форме конфигурации.
	[ ] Кнопка калибровки в графическом режиме работы с файлом конфигурации: 
		* вывод информационного окна.
		* набор массива позиций автоматически.
		* решение системы уравнений размером вплодь до 100х6.
	[-] Проверка на прототипе графического режима Compass.
		В настоящий момент не функционирует из-за изменившейся логики 
		протокола UART.
	[ ] Поле поправки дистанции в графическом режиме.
	[ ] Поле с углом магнитного вектора в графическом режиме.
	[+] Правка MAG_TILT в конфигурации (дублирование).
	[-] Закрыть 2 контакта на LCD CONN (нет эффекта).
	[-] Подогнуть эти 2 контакта (нет эффекта).
	[-] Попробовать экран со 2ого дальномера.
	[+] Подключить вывод LCD RESET, разобраться с сигналом на нем.
		Глюки встречаются и на USB-режиме, когда экран включается уже после 
		полной инициализации пинов в программе модуля.
	[+] Переработка модуля калибровки на меньшее использование памяти:
		* путем накопления значений в uint16_t.
		* (-)переход к float только на этапе работы с матрицей 6х6.
		* оптимизация матричных вычислений.
	[ ] Доработки "юзабилити".
	[+] Улучшение фильтраций и времени обновлений (особенно при калибровке).
	[ ] Восстановление функционирования утилиты.
	[ ] Расширение графического режима.
	[ ] Онлайн калибровка (в процессе работы).
	[ ] Вывод сервисной информации на экран.
	[ ] Подправить дескрипторы в программе дальномера.

	Утилита имеет 2 варианта сборки:
	1. Distrib - Версия для распространения. Основная утилита пользователя 
		для работы с устройством. В дальнейшем утилита будет разбита на несколько 
		составляющих. В настоящий момент ведется монолитная версия.
	2. Develop - Версия разработчика. Включает в себя секретный функционал, 
		включая ключи и логику шифрования образа и дополнительную вычислительну 
		логику. Версия разработчика собирается установкой параметра:
		-DDEVELOPER_BUILD=1
		* Нельзя допустить выход в Public данной версии утилиты!!!
	
	v 0.x
		* Предрелизные версии. Добавление функционала без ведения лога изменений.
		* Основной функционал работы с устройством.
		* Начальная поддержка работы с логами.
		* На данном этапе активное расширение функционала.
		
	v 1.0
		* Первая релизная версия утилиты.
		
	v 1.1
		* Полноценная реализация всего предполагаемого функционала.
		* Исправление обнаруженных ошибок.
		
	v 2.0 (future)
		* Глубокая переработка утилиты.
		* Разделение на несколько программ:
			- Консольная утилита работы с устройством.
			- Графическая утилита чтения и сохранения логов.
			- Графическая утилита получения измерений.
			- Графическая утилита работы с конфигурацией.
	
	igorkov | fsp@igorkov.org | vk.com/igorkov | 2013-2014
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <math.h>

#include "vector.h"
#include "io.h"
#include "crc.h"

#include "hid.h"
#include "file.h"

#include "orientation.h"
#include "solve.h"
#include "calibrate.h"

#include "gost.h"

#include "printf.h"
#include "draw.h"
#include "lowpass.h"


#if (DEVELOPER_BUILD != 0)
#include "lcd.h"
#endif

#if (DEVELOPER_BUILD != 0)
#include "private_key.h"
#endif

/*
	Текущая версия утилиты:
	00.00
		Данная версия отсутсвовала.
	00.01
		Версия с первыми наработками функционала.
		Функционал калибровки.
	00.02
		Активное расширение функционала:
		Добавление граф. режима.
		Начальная поодержка генерации логов.
		Поддержка механизмов Firmware Update.
	00.03
		Расширение функционала.
		Правка найденных ошибок.
		Начальная реализация графического режима редактирования конфигурации.
 */
#define VERSION     0x00
#define SUBVERSION  0x03

hid_context device;

#if (GRAPH_SUPPORT != 0)

#include <windows.h>
#include "resource.h"

BOOL CALLBACK ConfigDialogProc(HWND hwndDlg, 
						UINT message, 
						WPARAM wParam, 
						LPARAM lParam);

int graph_into(hid_context *device);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ProcGenerateFrame(LPVOID);
DWORD WINAPI ProcGetData(LPVOID);

char szClassName[] = "Compass";

#if 0
#define DBG printf 
#else
#define DBG(...) 
#endif

int winx = SIZE_X;
int winy = SIZE_Y;
uint32_t *frame_buffer = NULL;
HWND hWnd;
HANDLE hThread1;
HANDLE hThread2;

int fKillThread = 0;

typedef struct
{
	//... todo
	DWORD *frame_buffer;
	HWND hWnd;
} proc_params_t;

#endif // GRAPH_SUPPORT

#if 0
/*
	create_console()
	
	Функция создания консоли. 
	Можно использовать для создания консоли, для приложения собранного в виде Native (а не Console).
 */
void create_console(void) 
{ 
	FreeConsole();
	if ( AllocConsole() == 0 ) 
	{
		ExitProcess(2);
	}
	
	FILE *hf;
	hf = _fdopen(_open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), 2/*_O_TEXT*/), "w");
	*stdout = *stderr = *hf;
	return;
}
#endif

/*
	frw_load_256()
	Загрузка блока микропрограммы (256б).
 */
int frw_load_256(uint8_t *data, int len)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	int i;
	int lenb = 56;
	
	// Цепочка блоков 56+56+56+56+32
	for (i = 0; i < len; i += lenb)
	{
		if ((i + lenb) > len)
			lenb = (len-i);
		
		data_io->pc2dev.bOperation = OP_FW_WRITE_RAM;
		data_io->pc2dev.bDevAddr = 0;
		data_io->pc2dev.bAddr = 0;
		data_io->pc2dev.bLen = lenb;
		memcpy(data_io->pc2dev.abData, (uint8_t*)&data[i], lenb);
		
		ret = hid_write(&device, buffer, 64);
		if (ret == 0)
		{
			return -1;
		}
		
rep_read_answer_w:
		ret = hid_read(&device, buffer, 64);
		if (ret == 0)
		{
			return -1;
		}
		
		if (data_io->dev2pc.bRetStatus == 0xFF)
		{
			goto rep_read_answer_w;
		}
	}
	
	data_io->pc2dev.bOperation = OP_FW_WRITE_BLOCK;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w2:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w2;
	}

	return data_io->dev2pc.bRetStatus;
}

#if (DEVELOPER_BUILD == 1)
/*
	frw_read()
	Обратное чтение памяти контроллера.
	Отладочный функционал.
 */
int frw_read(uint8_t *data, uint32_t addr, int len)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	int i;
	int lenb = 56;
	
	// Цепочка блоков 56+56+56+56+32
	for (i = 0; i < len; i += lenb)
	{
		if ((i + lenb) > len) {
			lenb = (len-i);
		}
		
		data_io->pc2dev_read.bOperation = OP_FW_READ_BLOCK;
		data_io->pc2dev_read.bLen       = lenb;
		data_io->pc2dev_read.dwAddr     = addr+i;
		
		ret = hid_write(&device, buffer, 64);
		if (ret == 0)
		{
			return -1;
		}
		
rep_read_answer_w:
		ret = hid_read(&device, buffer, 64);
		if (ret == 0)
		{
			return -1;
		}
		
		if (data_io->dev2pc.bRetStatus == 0xFF)
		{
			goto rep_read_answer_w;
		}
		
		memcpy((uint8_t*)&data[i], data_io->dev2pc.abData, lenb);
	}
	
	return data_io->dev2pc.bRetStatus;
}
#endif

/*
	frw_get_hw()
	Получение аппаратной версии устройтсва.
 */
int frw_get_hw(uint32_t *hw_ver)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_FW_GET_HW;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	if (data_io->dev2pc.bLen != sizeof(uint32_t))
	{
		return -1;
	}
	*hw_ver = data_io->dev2pc_hw.hw_ver;
	
	return data_io->dev2pc.bRetStatus;
}

/*
	ee_read()
	Чтение EEPROM.
 */
int ee_read(uint16_t addr, uint8_t *data, uint8_t len)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	if (len > (64-8))
	{
		return -1;
	}
	
	data_io->pc2dev_ee.bOperation = OP_EEREAD;
	data_io->pc2dev_ee.bDevAddr = 0;
	data_io->pc2dev_ee.wAddr = addr;
	data_io->pc2dev_ee.bLen = len;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	memcpy((uint8_t*)data, data_io->dev2pc.abData, len);
	return data_io->dev2pc.bRetStatus;
}

/*
	ee_write()
	Запись EEPROM.
 */
int ee_write(uint16_t addr, uint8_t *data, uint8_t len)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	if (len > (64-8))
	{
		return -1;
	}
	
	data_io->pc2dev_ee.bOperation = OP_EEWRITE;
	data_io->pc2dev_ee.bDevAddr = 0;
	data_io->pc2dev_ee.wAddr = addr;
	data_io->pc2dev_ee.bLen = len;
	memcpy(data_io->pc2dev_ee.abData, (uint8_t*)data, len);

	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	return data_io->dev2pc.bRetStatus;
}

/*
	get_info()
	Получение структуры с информацией об устройстве.
 */
int get_info(pdevice_info_t device_info)
{
#if 0
	// Тестовый возврат структуры.
	device_info->frw_version_h = 0x01;
	device_info->frw_version_l = 0x00;
	device_info->serial_number = 0xAABBCCDD;
	device_info->memory_size   = 0x10000;
	device_info->log_len       = 1234;
	return 0;
#else
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_GET_INFO;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	*device_info = data_io->dev2pc_info.info;
	return data_io->dev2pc.bRetStatus;
#endif
}

/*
	erase_frw()
	Посылка команды стирания микропрограммы.
 */
int erase_frw(void)
{
	int ret;
	unsigned char buffer[64];
	unsigned char erase_vector[] = FW_CLEAR_VECTOR;
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_FW_CLEAR;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = sizeof(erase_vector);
	memcpy(data_io->pc2dev.abData, erase_vector, sizeof(erase_vector));
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	return data_io->dev2pc.bRetStatus;
}

/*
	erase_frw()
	Убиваем микропрограмму (стирание загрузчика).
 */
int kill_frw(void)
{
	int ret;
	unsigned char buffer[64];
	unsigned char erase_vector[] = FW_KILL_VECTOR;
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_FW_CLEAR;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = sizeof(erase_vector);
	memcpy(data_io->pc2dev.abData, erase_vector, sizeof(erase_vector));
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	return data_io->dev2pc.bRetStatus;
}

/*
	switch_frw()
	Переключение в режим загрузчика.
 */
int switch_frw(void)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_FW_SWITCH;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	return data_io->dev2pc.bRetStatus;
}

#if (DEVELOPER_BUILD != 0)
/*
	laser_on()
	Отладочная команда включения дальномера.
 */
int laser_on(void)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_LAS_POWER_ON;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	return data_io->dev2pc.bRetStatus;
}

/*
	laser_off()
	Отладочная команда выключения дальномера.
 */
int laser_off(void)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_LAS_POWER_OFF;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	return data_io->dev2pc.bRetStatus;
}

/*
	laser_en()
 */
int laser_en(void)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_LAS_ENABLE;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	return data_io->dev2pc.bRetStatus;
}

/*
	laser_lcd_send()
 */
int laser_lcd_send(uint8_t *frame, int len)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	if (len >= 48)
	{
		return -1;
	}
	
	data_io->pc2dev.bOperation = OP_LAS_LCD_SEND;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = len;
	
	// Копируем кадровый буфер:
	memcpy(data_io->pc2dev.abData, frame, len);
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	return data_io->dev2pc.bRetStatus;
}

/*
	laser_lcd_send_low()
 */
int laser_lcd_send_low(void)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_LAS_LCD_SEND_LOW;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	return data_io->dev2pc.bRetStatus;
}

#endif


/*
	get_raw_data()
	Получение сырых данных с датчиков.
 */
int get_raw_data(int op, pvector3d_t data)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = op;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	memcpy((uint8_t*)data, data_io->dev2pc.abData, sizeof(vector3d_t));
	return data_io->dev2pc.bRetStatus;
}

/*
	get_raw_data()
	Получение сырых данных с датчиков.
 */
uint8_t i2c(uint8_t stat, uint8_t data)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_I2C_SEND;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 2;
	data_io->pc2dev.abData[0] = stat;
	data_io->pc2dev.abData[1] = data;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFE)
		return data_io->dev2pc.bRetStatus;
	else
		return data_io->dev2pc.abData[0];
}

/*
	get_temp_data()
	Получение температуры.
	Данное значение доступно в магнитном датчике.
 */
int get_temp_data(uint16_t *temp)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_GET_LSM_TEMP;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	*temp = data_io->dev2pc_temp.temp;
	return data_io->dev2pc.bRetStatus;
}

/*
	get_config()
	Чтение конфигурации. Через штатную функцию EEPROM.
 */
int get_config(pconfig_t conf)
{
	int i;
	int ret;
	
	for (i = 0; i < sizeof(config_t); i += 32)
	{
		ret = ee_read(i, &(((uint8_t*)conf)[i]), 32);
		if (ret) break;
	}
	
	return ret;
}

/*
	get_log()
	Чтение лога.
 */
int get_log(plog_unit_t unit, int i)
{
	int ret;
	ret = ee_read(MEM_LOG_OFFSET+i*sizeof(log_unit_t), (uint8_t*)unit, sizeof(log_unit_t));
	return ret;
}

/*
	set_config()
	Установка конфигурации.
 */
int set_config(pconfig_t conf)
{
	int i;
	int ret;
	
	for (i=0; i<sizeof(config_t);i+=32)
	{
		ret = ee_write(i, &(((uint8_t*)conf)[i]), 32);
		if (ret) break;
	}
	
	return ret;
}

/*
	get_imu_data()
	Получение рассчитанной ориентации.
 */
int get_imu_data(pimu_data_t imu)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_GET_IMU;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	*imu = data_io->dev2pc_imu.imu;
	return data_io->dev2pc.bRetStatus;
}

/*
	get_distantion()
	Получение расстояния.
 */
int get_distantion(int *mm)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_GET_DIST;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	*mm = data_io->dev2pc_dist.mm;
	return data_io->dev2pc.bRetStatus;
}

/*
	get_distantion_start()
	Не блокирующая функция получения расстояния.
	Начало измерения.
 */
int get_distantion_start(void)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_GET_DIST_START;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	return data_io->dev2pc.bRetStatus;
}


/*
	get_distantion_end()
	Не блокирующая функция получения расстояния.
	Конец измерения, возврат данных.
 */
int get_distantion_end(int *mm)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_GET_DIST_END;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	if (data_io->dev2pc_dist.bLen == sizeof(int))
	{
		*mm = data_io->dev2pc_dist.mm;
	}
	return data_io->dev2pc.bRetStatus;
}

#if 0 // old logic
/*
	mean_calc()
	Вычисление среднего по массиву значений.
 */
void mean_calc(pvector3df_t mean, pvector3d_t arr, int k)
{
	int i;

	mean->x = 0.0f;
	mean->y = 0.0f;
	mean->z = 0.0f;
	
	for (i = 0; i < k; i++)
	{
		mean->x += arr[i].x;
		mean->y += arr[i].y;
		mean->z += arr[i].z;
	}

	mean->x /= k;
	mean->y /= k;
	mean->z /= k;
	
	return;
}
#endif

int str2int(char *str)
{
	int len = strlen(str);
	if ((len < 10) &&
		(str[len-1] == 'h' ||
		 str[len-1] == 'H'))
	{
		int i;
		int value = 0;
		
		for (i = 0; i < len-1; i++)
		{
			if ((str[i] <= 'F') && (str[i] >= 'A'))
			{
				value <<= 4;
				value += (str[i] + 10 - 'A');
			}
			else if ((str[i] <= 'f') && (str[i] >= 'a'))
			{
				value <<= 4;
				value += (str[i] + 10 - 'a');
			}
			else if ((str[i] <= '9') && (str[i] >= '0'))
			{
				value <<= 4;
				value += (str[i] - '0');
			}
			else
			{
				// nop
				printf("WARNING: Incorrect symbol \"%c\" in hex-value!\r\n", str[i]);
			}
		}
		return value;
	}
	else
	{
		return atoi(str);
	}
}

/* Прототипы функций: */
void help(void);
void print_log(FILE *f, plog_unit_t log, int n);
void print_log_topos(FILE *f, plog_unit_t log, int n);
void print_config(FILE *f, pconfig_t conf);
void printf_buffer(FILE *f, uint16_t addr, uint8_t *data, int len);

/* Образ микропрограммы для загрузки: */
#if (DEVELOPER_BUILD != 0)
extern uint8_t loader_bin[];
extern uint32_t loader_len;
#endif

/*
	Режимы работы. Режим выбирается при парсинге командной строки.
 */
#define MODE_UND                 (-1)
#define MODE_SET_CONFIG          0x01
#define MODE_ERASE_CONFIG        0x02
#define MODE_ERASE_LOG           0x03
#define MODE_GET_INFO            0x04
#define MODE_GET_CONFIG          0x05
#define MODE_GET_LOG             0x06
#define MODE_GET_IMU             0x07
#define MODE_GET_DIST            0x08
#define MODE_GRAPH_COMPASS       0x09
#define MODE_GRAPH_CONFIG        0x0A

#define MODE_GET_MEMORY          0x0B
#define MODE_SET_MEMORY          0x0C
#define MODE_GET_RAW             0x0D
#define MODE_ERASE_FRW           0x0E
#define MODE_ERASE_FRWF          0x0F

#define MODE_CALC_CALIBRATION    0x10
#define MODE_CALC_CALIBRATIONA   0x11
#define MODE_CALC_ORIENTATION    0x12

#define MODE_SET_FRW             0x13
#define MODE_SET_FRWF            0x14
#define MODE_CALC_FRW            0x15
#define MODE_TEST_FRW            0x16

#define MODE_LOADER              0x17

#define MODE_GET_TEMP            0x18

/* Режимы работы с лазерным дальномером: */
#define MODE_LASER_ON            0x19
#define MODE_LASER_OFF           0x1A
#define MODE_LASER_EN            0x1B

/* Тест LCD: */
#define MODE_TEST_LCD            0x1C

#define MODE_READ_ISP            0x1D

/*
	Форматы сохранения во внешние файлы.

	UND
		Undefined, формат не задан.
	TEXT
		Текстовый вывод.
	BINARY
		Бинарный вывод.
	CSV
		В виде файла CSV.
	TOPOS
		В формате программы Topos.
	HEX
		В виде HEX-файла.
	
	Формат выбирается при парсинге командной строки.
 */
#define FORMAT_UND               (-1)
#define FORMAT_TEXT              0x01
#define FORMAT_BINARY            0x02
#define FORMAT_CSV               0x03
#define FORMAT_TOPOS             0x04
#define FORMAT_HEX               0x05

int main(int argc, char **argv)
{
	int i = 1;
	int ret = 0;
	int rep = -1;
	
	// mode
	int mode = MODE_UND;
	// console flag:
	int consf = 1;
	// bt flag
	int btf = 0;
	int btport = -1;
	// positions
	int pos_start = -1;
	int pos_end = -1;
#if (DEVELOPER_BUILD != 0)
	// id
	int id = -1;
	char disk = ' ';
#endif
	// format
	int format = FORMAT_UND;
	// files
	char *file_out = NULL;
	char *file_in = NULL;
	
	// Check data struct:
	if (sizeof(config_t) != CONFIG_SIZE)
	{
		printf("ERROR: Incorrect building, sizeof(config_t) = %d!\r\n", (int)sizeof(config_t));
		return 0;
	}
	
	// Определяем как запущена программа:
	if (1)
	{
		HANDLE hcons;
		CONSOLE_SCREEN_BUFFER_INFO cons_info;
		
		hcons = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(hcons, &cons_info);
		
		if (cons_info.dwCursorPosition.X == 0 &&
			cons_info.dwCursorPosition.Y == 0)
		{
			// Запущено из GUI.
			consf = 0;
		}
		
		// @note
		// Данное определение работает не точно.
		// Во многих случаях при запуске может ошибочно определяться, 
		// что работает графический режим.
	}
	
	// Запущена без параметров?
	if (argc == 1)
	{
		help();
		
		if (consf == 0)
		{
#if 0 && (GRAPH_SUPPORT != 0)
			printf("WARNING: Run from GUI! Graphical mode set automatically.\r\n");
			mode = MODE_GRAPH_COMPASS;
#else
			// Ожидаем нажатие клавиши, чтобы пользователь увидел сообщение:
			getch();
#endif
		}
		goto exit;
	}

	// Обработка входных параметров:
	while (i<argc)
	{
		if (_stricmp(argv[i], "-help") == 0)
		{
			help();
			goto exit;
		}
		else if (_stricmp(argv[i], "-info") == 0)
		{
			// Получение общей информации.
			// Дублирование ветки -get info
			mode = MODE_GET_INFO;
		}
		else if (_stricmp(argv[i], "-read") == 0)
		{
			// Получение данных.
			// Дублирование ветки -get info
			mode = MODE_READ_ISP;
		}
		else if (_stricmp(argv[i], "-get") == 0)
		{
			if (mode != MODE_UND)
			{
				printf("ERROR: Prameter error (more one mode)!\r\n");
				//help();
				return 0;
			}
			i++;
			if ((i < argc) && (argv[i][0] != '-'))
			{
				if (_stricmp(argv[i], "info") == 0)
				{
					// Получение общей информации.
					mode = MODE_GET_INFO;
				}
				else if (_stricmp(argv[i], "log") == 0)
				{
					mode = MODE_GET_LOG;
				}
				else if (_stricmp(argv[i], "config") == 0)
				{
					mode = MODE_GET_CONFIG;
				}
				else if (_stricmp(argv[i], "imu") == 0)
				{
					mode = MODE_GET_IMU;
				}
				else if (_stricmp(argv[i], "dist") == 0)
				{
					mode = MODE_GET_DIST;
				}
#if (DEVELOPER_BUILD != 0)
				else if (_stricmp(argv[i], "memory") == 0)
				{
					mode = MODE_GET_MEMORY;
				}
				else if (_stricmp(argv[i], "raw") == 0)
				{
					mode = MODE_GET_RAW;
				}
				else if (_stricmp(argv[i], "temp") == 0)
				{
					mode = MODE_GET_TEMP;
				}
#endif
				else
				{
					printf("ERROR: Unknown argument \"%s %s\"!\r\n", argv[i-1], argv[i]);
					//help();
					return 0;
				}
			}
			else
			{
				printf("ERROR: Arguments error!\r\n\r\n");
				//help();
				return 0;
			}
		}
		else if (_stricmp(argv[i], "-set") == 0)
		{
			if (mode != MODE_UND)
			{
				printf("ERROR: Prameter error (more one mode)!\r\n");
				//help();
				return 0;
			}
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				if (_stricmp(argv[i], "config") == 0)
				{
					mode = MODE_SET_CONFIG;
				}
#if (DEVELOPER_BUILD != 0)
				else if (_stricmp(argv[i], "memory") == 0)
				{
					mode = MODE_SET_MEMORY;
				}
#endif
				else if (_stricmp(argv[i], "firmware") == 0)
				{
					mode = MODE_SET_FRW;
				}
				else
				{
					printf("ERROR: Unknown argument \"%s %s\"!\r\n", argv[i-1], argv[i]);
					//help();
					return 0;
				}
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
#if (DEVELOPER_BUILD != 0)
		else if (_stricmp(argv[i], "-loader") == 0)
		{
			if (mode != MODE_UND)
			{
				printf("ERROR: Prameter error (more one mode)!\r\n");
				//help();
				return 0;
			}
			mode = MODE_LOADER;
		}
#endif
		else if (_stricmp(argv[i], "-erase") == 0)
		{
			if (mode != MODE_UND)
			{
				printf("ERROR: Parameter error (more one mode)!\r\n");
				//help();
				return 0;
			}
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				if (_stricmp(argv[i], "config") == 0)
				{
					mode = MODE_ERASE_CONFIG;
				}
				else if (_stricmp(argv[i], "log") == 0)
				{
					mode = MODE_ERASE_LOG;
				}
#if (DEVELOPER_BUILD != 0)
				else if (_stricmp(argv[i], "firmware") == 0)
				{
					mode = MODE_ERASE_FRW;
				}
				else if (_stricmp(argv[i], "loader") == 0)
				{
					mode = MODE_ERASE_FRWF;
				}
#endif
				else
				{
					printf("ERROR: Unknown argument \"%s %s\"!\r\n", argv[i-1], argv[i]);
					//help();
					return 0;
				}
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
		else if (_stricmp(argv[i], "-graph") == 0)
		{
			if (mode != MODE_UND)
			{
				printf("ERROR: Prameter error (more one mode)!\r\n");
				//help();
				return 0;
			}
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				if (_stricmp(argv[i], "compass") == 0)
				{
					mode = MODE_GRAPH_COMPASS;
				}
				else if (_stricmp(argv[i], "config") == 0)
				{
					mode = MODE_GRAPH_CONFIG;
				}
				else
				{
					printf("ERROR: Unknown argument \"%s %s\"!\r\n", argv[i-1], argv[i]);
					//help();
					return 0;
				}
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
		else if (_stricmp(argv[i], "-start") == 0)
		{
			if (pos_start != -1)
			{
				printf("ERROR: Position resetup!!\r\n");
				//help();
				return 0;
			}
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				pos_start = str2int(argv[i]);
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
		else if (_stricmp(argv[i], "-end") == 0)
		{
			if (pos_end != -1)
			{
				printf("ERROR: Position resetup!!\r\n");
				//help();
				return 0;
			}
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				pos_end = str2int(argv[i]);
			}
			else
			{
				printf("Arguments error!\r\n");
				//help();
				return 0;
			}
		}
#if (DEVELOPER_BUILD != 0)
		else if (_stricmp(argv[i], "-id") == 0)
		{
			if (id != -1)
			{
				printf("ERROR: ID resetup!!\r\n");
				//help();
				return 0;
			}
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				id = str2int(argv[i]);
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
#endif
#if (DEVELOPER_BUILD != 0)
		else if (_stricmp(argv[i], "-disk") == 0)
		{
			if (disk != ' ')
			{
				printf("ERROR: Disk resetup!!\r\n");
				//help();
				return 0;
			}
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				disk = argv[i][0];
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
#endif
		else if (_stricmp(argv[i], "-format") == 0)
		{
			if (format != -1)
			{
				printf("ERROR: Format resetup!!\r\n");
				//help();
				return 0;
			}
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				if (_stricmp(argv[i], "text") == 0)
				{
					format = FORMAT_TEXT;
				}
				else if (_stricmp(argv[i], "binary") == 0)
				{
					format = FORMAT_BINARY;
				}
				else if (_stricmp(argv[i], "csv") == 0)
				{
					format = FORMAT_CSV;
				}
				else if (_stricmp(argv[i], "topos") == 0)
				{
					format = FORMAT_TOPOS;
				}
				else if (_stricmp(argv[i], "hex") == 0)
				{
					format = FORMAT_HEX;
				}
				else
				{
					printf("ERROR: Unknown argument \"%s %s\"!\r\n", argv[i-1], argv[i]);
					//help();
					return 0;
				}
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
		else if (_stricmp(argv[i], "-in") == 0)
		{
			if (file_in != NULL)
			{
				printf("ERROR: Input file multiple define!\r\n");
				//help();
				return 0;
			}
			i++;
			if (i < argc)
			{
				file_in = argv[i];
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
		else if (_stricmp(argv[i], "-out") == 0)
		{
			if (file_out != NULL)
			{
				printf("ERROR: Input file multiple define!\r\n");
				//help();
				return 0;
			}
			i++;
			if (i < argc)
			{
				file_out = argv[i];
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
#if (DEVELOPER_BUILD != 0)
		else if (_stricmp(argv[i], "-calc") == 0)
		{
			if (mode != MODE_UND)
			{
				printf("ERROR: Parameter error (more one mode)!\r\n");
				//help();
				return 0;
			}
			
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				if (_stricmp(argv[i], "orientation") == 0)
				{
					mode = MODE_CALC_ORIENTATION;
				}
				else if (_stricmp(argv[i], "calibration") == 0)
				{
					mode = MODE_CALC_CALIBRATION;
				}
				else
				{
					printf("ERROR: Unknown argument \"%s\"\r\n", argv[i]);
				}
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
#endif
#if (DEVELOPER_BUILD != 0)
		else if (_stricmp(argv[i], "-frw") == 0)
		{
			if (mode != MODE_UND)
			{
				printf("ERROR: Parameter error (more one mode)!\r\n");
				//help();
				return 0;
			}
			
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				if (_stricmp(argv[i], "erase") == 0)
				{
					mode = MODE_ERASE_FRW;
				}
				else if (_stricmp(argv[i], "kill_loader") == 0)
				{
					mode = MODE_ERASE_FRWF;
				}
				else if (_stricmp(argv[i], "update") == 0)
				{
					mode = MODE_SET_FRW;
				}
				else if (_stricmp(argv[i], "generate") == 0)
				{
					mode = MODE_CALC_FRW;
				}
				else if (_stricmp(argv[i], "test") == 0)
				{
					mode = MODE_TEST_FRW;
				}
				else
				{
					printf("ERROR: Unknown argument \"%s\"\r\n", argv[i]);
				}
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
#endif
#if (DEVELOPER_BUILD != 0)
		else if (_stricmp(argv[i], "-laser") == 0)
		{
			if (mode != MODE_UND)
			{
				printf("ERROR: Parameter error (more one mode)!\r\n");
				//help();
				return 0;
			}
			
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				if (_stricmp(argv[i], "on") == 0)
				{
					mode = MODE_LASER_ON;
				}
				else if (_stricmp(argv[i], "off") == 0)
				{
					mode = MODE_LASER_OFF;
				}
				else if (_stricmp(argv[i], "en") == 0)
				{
					mode = MODE_LASER_EN;
				}
				else if (_stricmp(argv[i], "lcd") == 0)
				{
					mode = MODE_TEST_LCD;
				}
				else
				{
					printf("ERROR: Unknown argument \"%s\"\r\n", argv[i]);
				}
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
		}
#endif
#if (BT_SUPPORT != 0)
		else if (_stricmp(argv[i], "-bt") == 0)
		{
			if (mode == MODE_UND)
			{
				printf("ERROR: Set parameter -bt last!\r\n");
				//help();
				return 0;
			}
			if (mode != MODE_CALC_CALIBRATION)
			{
				printf("ERROR: Bluetooth didn't support in this mode!\r\n");
				//help();
				return 0;
			}
			i++;
			if ((i < argc) && argv[i][0] != '-')
			{
				btport = str2int(argv[i]);
			}
			else
			{
				printf("ERROR: Arguments error!\r\n");
				//help();
				return 0;
			}
			
			btf = 1;
			mode = MODE_CALC_CALIBRATIONA;
		}
#endif
		else
		{
			printf("ERROR: Unknown argument \"%s\"!\r\n", argv[i]);
			//help();
			return 0;
		}
		i++;
	}
	
	if (btf == 0)
	{
repeate_connect:
		// Проверяем, что данный режим предусматривает наличие устройства:
		if (mode != MODE_UND &&
			mode != MODE_TEST_FRW && 
			mode != MODE_CALC_FRW &&
			mode != MODE_LOADER /*&&
			mode != MODE_GRAPH_CONFIG*/)
		{
			ret = 0;
			if (mode == MODE_SET_FRW ||
				mode == MODE_SET_FRWF ||
				mode == MODE_ERASE_FRWF ||
				mode == MODE_READ_ISP)
			{
				printf("Open FWU device... ");
				ret = hid_find(&device, 0x1FC9, 0x0031, 0xFF00);
				if (ret == 0)
				{
					// Зануляем hDeviceHandle.
					device.hDeviceHandle = NULL;
					// Не найдено!
					printf("not found!\r\n");
					if (rep > 0)
					{
						// nop
					}
					else
					{
						if (mode == MODE_ERASE_FRWF)
						{
							// для режима KILL не пытаемся переключиться.
							return 0;
						}
						mode = MODE_SET_FRWF;
						goto connect_std;
					}
				}
				else
				{
					printf("OK!\r\n");
				}
			}
			else
			{
connect_std:
				printf("Open STD device... ");
				ret = hid_find(&device, 0x1FC9, 0x0007, 0xFF00);
				
				if (ret == 0)
				{
					// Зануляем hDeviceHandle.
					device.hDeviceHandle = NULL;
					// Не найдено:
					printf("\r\n");
					if (mode == MODE_GRAPH_CONFIG)
					{
						printf("\r\n");
						printf("WARNING: Device not found!\r\n");
					}
					else
					{
						printf("\r\n");
						printf("ERROR: Device not found!\r\n");
						return 0;
					}
				}
				else
				{
					printf("OK!\r\n");
				}
			}
			
			if (consf == 0)
			{
				getch();
			}
		}
	}
	else // btf == 1
	{
		// Bluetooth
		// TODO
		
		// 1. open btport
		
		// 2. switch to BT mode
		
		// 3. command switch
		switch (mode)
		{
		case MODE_GET_INFO:
		case MODE_GET_RAW:
		case MODE_CALC_CALIBRATION:
		case MODE_CALC_CALIBRATIONA:
		
		default:
			printf("todo");
		}
		
		// 4. close bt port:
		
		return 0;
	}
	
	switch (mode)
	{
	case MODE_GET_INFO:
		{
			device_info_t device_info;
			ret = get_info(&device_info);
			if (ret)
			{
				printf("ERROR: get_info() return %d\r\n", ret);
				goto close_and_exit;
			}
			else
			{
				// Вывод на экран информации об устройстве:
				printf("Device info:\r\n");
				printf("Firmware version: %d.%d\r\n", device_info.frw_version_h, device_info.frw_version_l);
				printf("Hardware version: %08x\r\n", device_info.hrw_version);
				printf("Serial Number: %08X\r\n", device_info.serial_number);
				printf("Memory size: %db\r\n", device_info.memory_size);
				printf("Log size: %d samples\r\n", (device_info.memory_size-MEM_LOG_OFFSET)/(int)sizeof(log_unit_t));
				printf("Log filling: %d/%d (%d%%)\r\n", device_info.log_len, (device_info.memory_size-MEM_LOG_OFFSET)/(int)sizeof(log_unit_t), 
														(device_info.log_len*100)/((device_info.memory_size-MEM_LOG_OFFSET)/(int)sizeof(log_unit_t)));
				printf("Configuration: %s\r\n", device_info.configure ? "OK" : "Incorrect");
				if (device_info.error == 0)
					printf("Internal errors: none\r\n");
				else
					printf("Internal errors: %d\r\n", device_info.error);
					
				// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
				// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
				// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
				if (0)
				{
					#define I2C_START        0x01
					#define I2C_RESTART      0x02
					#define I2C_STOP         0x03
					#define I2C_ADDRESS      0x04
					#define I2C_TRANSMIT     0x05
					#define I2C_RECEIVE_ACK  0x06
					#define I2C_RECEIVE_NACK 0x07
					// mag test read
					//printf("00: %02x (I2C_STOP)\r\n", i2c(I2C_STOP, 0));
					
					#define ADDR 0x1E //0x19
					
					//printf("01: %02x (I2C_START)\r\n", i2c(I2C_START, 0));
					//printf("08: %02x (I2C_STOP)\r\n", i2c(I2C_STOP, 0));
					
					printf("01: %02x (I2C_START)\r\n", i2c(I2C_START, 0));
					printf("02: %02x (I2C_ADDRESS)\r\n", ret = i2c(I2C_ADDRESS, (ADDR<<1)&0xFE/*0x3C&0xFE*/));
					//if (ret == 0x18)
					{
						printf("03: %02x (I2C_TRANSMIT)\r\n", i2c(I2C_TRANSMIT, 0x03));
						printf("04: %02x (I2C_RESTART)\r\n", i2c(I2C_RESTART, 0));
						printf("05: %02x (I2C_ADDRESS)\r\n", i2c(I2C_ADDRESS, (ADDR<<1)|1/*0x3C|0x01*/));
						printf("06: %02x (I2C_RECEIVE_ACK)\r\n", i2c(I2C_RECEIVE_ACK, 0));
						printf("07: %02x (I2C_RECEIVE_NACK)\r\n", i2c(I2C_RECEIVE_NACK, 0));
					}
					printf("08: %02x (I2C_STOP)\r\n", i2c(I2C_STOP, 0));
				}
				// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
				// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
				// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
			}
		}
		goto close_and_exit;
	case MODE_SET_FRWF:
		{
			// В начале выключаем лазер:
			laser_off();
			
			// Первый этап:
			if (rep == -1)
			{
				rep = 5;
				
				// Смена режима на загрузчик:
				ret = switch_frw();
				if (ret)
				{
					printf("WARNING: switch_frw() return %d!\r\n", ret);
				}
			}
			else if (rep == 0)
			{
				goto close_and_exit;
			}
			
			// В момент переключения на загрузчик возможны задержки определения устройства в ОС
			// из-за установки драйверов. Вводим ожидание и повтор попытки подключения.
			printf("Waiting (5 sec)...\r\n");
			Sleep(5000);
			rep--;
			
			// Следующий этап:
			mode = MODE_SET_FRW;
			
			// Закрываем текущее устройство:
			hid_close(&device);
		}
		goto repeate_connect;
	case MODE_SET_FRW:
		{
			// Начинаем обмен:
			if (file_in == NULL)
			{
				printf("ERROR: Cannot open file \"%s\"!\r\n", file_out);
			}
			else
			{
				uint8_t *buffer;
				uint32_t len;
				uint32_t n;
				uint32_t hw_ver;
				pimg_t pimg;
				
				// Загружаем образ программы:
				ret = f2b(file_in, &buffer, &len);
				if (ret)
				{
					printf("ERROR: Cannot open file \"%s\"!\r\n", file_in);
					goto close_and_exit;
				}
				
				pimg = (pimg_t)buffer;
				
				// Проверка вектора в образе, чтобы не начать парсить случайный файл:
				if (pimg->tag != 0x73EB8A3B)
				{
					printf("ERROR: Incorrect firmware file!\r\n");
					goto close_and_exit;
				}
				
				// Получение аппаратной версии устройства:
				ret = frw_get_hw(&hw_ver);
				if (ret)
				{
					printf("ERROR: frw_get_hw() return %d!\r\n", ret);
					goto close_and_exit;
				}
				
				// Проверка, что образ предназначен для текущей аппаратной версии:
				if (pimg->hw_ver != hw_ver)
				{
					printf("ERROR: Incorrect HW-version for this firmware update file (need: %08x, file: %08x)!\r\n", pimg->hw_ver, hw_ver);
					goto close_and_exit;
				}
				
				// Проверяем CRC32 данных образа (зашифрованный блок):
				if (pimg->crc32 != crc32(0x00000000, pimg->data, pimg->size))
				{
					printf("ERROR: Incorrect CRC32 in firmware update!\r\n");
					goto close_and_exit;
				}
				
				// Стираем старый образ:
				printf("Erase firmware...");
				ret = erase_frw();
				if (ret)
				{
					printf("\r\nERROR: erase_frw() return %d!\r\n", ret);
					goto close_and_exit;
				}
				printf(" OK!\r\n");
				
				// Отправка программы блоками по 256 байт, блоки из образа отправляются "как есть",
				// расшифровка и проверка осуществляется на стороне микропрограммы ключа:
				printf("Write blocks");
				for (n = 0; n < pimg->size; n += 256)
				{
					int len = (pimg->size - n) > 256 ? 256 : (pimg->size - n);
					
					ret = frw_load_256(&pimg->data[n], len);
					if (ret)
					{
						free(buffer);
						printf("\r\n");
						printf("ERROR: frw_load_256() return %d!\r\n", ret);
						goto close_and_exit;
					}
					else
					{
						printf(".");
					}
				}
				printf(" OK\r\n");
				
				// Сообщение о необходимости перезапуска:
				printf("Please restart device!\r\n");
				
				free(buffer);
			}
		}
		goto close_and_exit;
#if (DEVELOPER_BUILD != 0)
	case MODE_ERASE_FRW:
		{
			// Команда на стирание программы, с определенного момента данная команда может не поддерживаться:
			ret = erase_frw();
			if (ret == -1)
			{
				printf("Please reconnect device!\r\n");
			}
			else if (ret)
			{
				printf("ERROR: erase_frw() return %d!\r\n", ret);
			}
		}
		goto close_and_exit;
	case MODE_ERASE_FRWF:
		{
			// Команда на стирание программы, с определенного момента данная команда может не поддерживаться:
			printf("kill_frw()...\r\n");
			ret = kill_frw();
			if (ret == -1)
			{
				printf("Please reconnect device!\r\n");
			}
			else if (ret)
			{
				printf("ERROR: erase_frw() return %d!\r\n", ret);
			}
		}
		goto close_and_exit;
#endif
#if (DEVELOPER_BUILD != 0)
	case MODE_TEST_FRW:
		{
			// Функционал проверки криптоблока.
			// Шифрование/расшифрование тестовых данных.
			// При наличие входного образа, его тестирование.
			
			#define GOST_TEST_SIZE 128
			
			uint8_t key[32] = { 
					0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
					0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, };
			uint8_t plain[GOST_TEST_SIZE];
			uint8_t result[GOST_TEST_SIZE];
			
			uint32_t cbc1 = 0x12345678;
			uint32_t cbc2 = 0x87654321;
			
			int i;
			
			gost_init((uint32_t*)key);
			
			printf("Plain buffer:\r\n");
			for (i = 0; i < GOST_TEST_SIZE; i++)
			{
				plain[i] = rand();
				result[i] = plain[i];
			}
			printf_buffer(stdout, 0, plain, GOST_TEST_SIZE);
			
			printf("Crypted buffer:\r\n");
			gost_crypt_cbc((uint32_t*)result, GOST_TEST_SIZE>>2, cbc1, cbc2);
			printf_buffer(stdout, 0, result, GOST_TEST_SIZE);
			
			printf("Uncrypted buffer:\r\n");
			gost_decrypt_cbc((uint32_t*)result, GOST_TEST_SIZE>>2, cbc1, cbc2);
			printf_buffer(stdout, 0, result, GOST_TEST_SIZE);
			
			if (memcmp(plain, result, GOST_TEST_SIZE) == 0)
			{
				printf("GOST 28147-89 test ok!\r\n");
			}
			else
			{
				printf("ERROR: GOST 28147-89 test error!\r\n");
			}

#if 0
			// for future realization
			if (file_in != NULL)
			{
				uint8_t *buffer;
				uint32_t len;
				ret = f2b(file_in, &buffer, &len);
				if (ret)
				{
					printf("ERROR: Cannot open file \"%s\"!\r\n", file_in);
					goto close_and_exit;
				}
				
				// тестирование образа
				// TODO
				
				
				free(buffer);
			}
#endif
		}
		goto exit;
#endif
#if (DEVELOPER_BUILD != 0)
	case MODE_GET_MEMORY:
		if (pos_start == -1)
		{
			pos_start = 0;
			printf("WARNING: Start position no set (new value = %d).\r\n", pos_start);
		}
		if (pos_end == -1)
		{
			//pos_end = 0x10000-1;
			printf("ERROR: End position not set!\r\n");
			goto close_and_exit;
		}
		if (file_out == NULL)
		{
			uint8_t data[48];
			
			printf("Memory dump:\r\n");
			for (i = pos_start; i <= pos_end; i += 48)
			{
				int len = (pos_end - i) < 48 ? (pos_end - i) : 48;
				ret = ee_read(i,  data, len);
				if (ret)
				{
					printf("ERROR: ee_read() return %d\r\n", ret);
					goto close_and_exit;
				}
				printf_buffer(stdout, i, data, len);
			}
		}
		else
		{
			printf("Memory dump to file %s\r\n", file_out);
			printf("TODO!\r\n");
		}
		goto close_and_exit;
#endif
#if (DEVELOPER_BUILD != 0)
	case MODE_SET_MEMORY:
		{
			uint8_t *buffer;
			uint32_t len;
			if (pos_start == -1)
			{
				//pos_start = 0;
				printf("ERROR: Start position no set.\r\n");
				goto close_and_exit;
			}
			ret = f2b(file_in, &buffer, &len);
			if (ret)
			{
				printf("ERROR: File \"%s\" open error!\r\n", file_in);
				goto close_and_exit;
			}
			else
			{
				printf("Dump file \"%s\" loaded!\r\n", file_in);
			}
			if (pos_end != -1)
			{
				if ((pos_end-pos_start) > len)
				{
					printf("ERROR: End position more that file size!\r\n");
					goto close_and_exit;
				}
				else
				{
					//len = (pos_end-pos_start);
				}
			}
			printf("Memory write...\r\n");
			for (i = pos_start; i <= pos_end; i += 32)
			{
				int len = (pos_end - i) < 32 ? (pos_end - i) : 32;
				ret = ee_write(i,  &buffer[i-pos_start], len);
				if (ret)
				{
					printf("ERROR: ee_write() return %d\r\n", ret);
					goto close_and_exit;
				}
				printf_buffer(stdout, i, &buffer[i-pos_start], len);
			}
			printf("OK!\r\n");
		}
		goto close_and_exit;
#endif
#if (DEVELOPER_BUILD != 0)
	case MODE_GET_RAW:
		{
			vector3d_t mag;
			vector3d_t acc;
			
			ret = get_raw_data(OP_GET_LSM_ACC, &acc);
			if (ret)
			{
				printf("ERROR: get_raw_data() return %d\r\n", ret);
				goto close_and_exit;
			}
			ret = get_raw_data(OP_GET_LSM_MAG, &mag);
			if (ret)
			{
				printf("ERROR: get_raw_data() return %d\r\n", ret);
				goto close_and_exit;
			}
			
			printf("Sensors RAW data:\r\n");
			printf("ACC: (%d, %d, %d)\r\n", acc.x, acc.y, acc.z);
			printf("MAG: (%d, %d, %d)\r\n", mag.x, mag.y, mag.z);
		}
		goto close_and_exit;
#endif
#if (DEVELOPER_BUILD != 0)
	case MODE_GET_TEMP:
		{
			uint16_t temp;
			
			ret = get_temp_data(&temp);
			if (ret)
			{
				printf("ERROR: get_temp_data() return %d\r\n", ret);
				goto close_and_exit;
			}
			
			printf("Temperature sensor data: %d\r\n", temp);
		}
		goto close_and_exit;
#endif
	case MODE_ERASE_CONFIG:
		{
			config_t conf;
			memset((uint8_t*)&conf, 0xFF, sizeof(config_t));
			ret = set_config(&conf);
			if (ret)
			{
				printf("ERROR: set_config() return %d!\r\n", ret);
			}
			else
			{
				printf("Configuration erased!\r\n");
			}
		}
		goto close_and_exit;
	case MODE_ERASE_LOG:
		{
			log_unit_t unit;
			device_info_t device_info;
			pos_start = 0;
			ret = get_info(&device_info);
			if (ret)
			{
				printf("ERROR: get_info() return %d\r\n", ret);
				goto close_and_exit;
			}
			pos_end = (device_info.memory_size-MEM_LOG_OFFSET)/sizeof(log_unit_t)-1;
			memset((uint8_t*)&unit, 0xFF, sizeof(log_unit_t));
			for (i = pos_start; i <= pos_end; i++)
			{
				ret = ee_write(MEM_LOG_OFFSET+i*sizeof(log_unit_t), (uint8_t*)&unit, sizeof(log_unit_t));
				if (ret)
				{
					printf("ERROR: ee_write() return %d!\r\n", ret);
					goto close_and_exit;
				}
				printf("Delete %d/%d\r", i, pos_end);
			}
			printf("Log file deleted!\r\n");
		}
		goto close_and_exit;
	case MODE_GET_CONFIG:
		{
			config_t conf;
			ret = get_config(&conf);
			if (ret)
			{
				printf("ERROR: get_config() return %d\r\n", ret);
				goto close_and_exit;
			}
			if (file_out == NULL &&
				(format == FORMAT_BINARY || 
				 format == FORMAT_UND))
			{
				printf("Config dump:\r\n");
				printf_buffer(stdout, 0, (uint8_t*)&conf, sizeof(config_t));
			}
			else if (file_out == NULL &&
					 format == FORMAT_TEXT)
			{
				print_config(stdout, &conf);
			}
			else
			{
				if (format == FORMAT_TEXT)
				{
					FILE *f;
					f = fopen (file_out,"wb");
					if (f == NULL)
					{
						printf("ERROR: saving error, fopen() return NULL!\r\n");
					}
					print_config(f, &conf);
					fclose (f);
				}
				else if ((format == FORMAT_BINARY) ||
						(format == FORMAT_UND))
				{
					ret = b2f(file_out, (uint8_t*)&conf, sizeof(config_t));
					if (ret)
					{
						printf("ERROR: saving error, b2f() return %d!\r\n", ret);
					}
					else
					{
						printf("Config dump saved to file \"%s\".\r\n", file_out);
					}
				}
				else
				{
					printf("ERROR: Config incorrect format!\r\n");
					goto close_and_exit;
				}
			}
		}
		goto close_and_exit;
	case MODE_GET_LOG:
		{
			log_unit_t unit;
			uint8_t *buffer;
			device_info_t device_info;
			if (pos_start == -1)
			{
				//pos_start = 0;
				printf("ERROR: Start position no set.\r\n");
				goto close_and_exit;
			}
			if (pos_end == -1)
			{
				pos_end = pos_start;
				printf("WARNING: End position no set, read one element.\r\n");
				//goto close_and_exit;
			}
			ret = get_info(&device_info);
			if (ret)
			{
				printf("ERROR: get_info() return %d\r\n", ret);
				goto close_and_exit;
			}
			if (pos_start >= (device_info.memory_size-MEM_LOG_OFFSET)/sizeof(log_unit_t) ||
				pos_end >= (device_info.memory_size-MEM_LOG_OFFSET)/sizeof(log_unit_t))
			{
				printf("ERROR: Log position out of range!\r\n");
				goto close_and_exit;
			}
			if (file_out != NULL)
			{
				buffer = (uint8_t*)malloc(sizeof(log_unit_t)*(pos_end-pos_start+1));
			}
			for (i = pos_start; i <= pos_end; i++)
			{
				uint8_t crc_rom;
				uint8_t crc_real;
				
				ret = get_log(&unit, i);
				if (ret)
				{
					printf("ERROR: get_log() return %d!\r\n", ret);
					goto close_and_exit;
				}
				crc_rom = unit.crc8;
				unit.crc8 = 0xFF;
				crc_real = crc8((uint8_t*)&unit, sizeof(log_unit_t));
				if (unit.type == 0xFF && unit.crc8 == 0xFF &&
					unit.id == 0xFFFF)
				{
					printf("LOG(%04d): NO DATA!\r\n", i);
				}
				else if (file_out == NULL)
				{
					printf("LOG(%04d):\r\n", i);
					printf("\tType:    %08X\r\n", unit.type);
					printf("\tCrc:     %08X (%s)\r\n", unit.crc8, (crc_rom == crc_real) ? "correct" : "incorrect");
					printf("\tID:      %d\r\n", unit.id);
					printf("\tDist:    %d\r\n", unit.dist);
					printf("\tTilt:    %.1f\r\n", unit.tilt / 10.0f);
					printf("\tAzimuth: %.1f\r\n", unit.azimuth / 10.0f);
				}
				else
				{
					printf("Read log %d/%d\r", i-pos_start, pos_end-pos_start);
					memcpy(&buffer[i*sizeof(log_unit_t)], (uint8_t*)&unit, sizeof(log_unit_t));
				}
			}
			if (file_out != NULL)
			{
				if (format == FORMAT_UND || format == FORMAT_BINARY)
				{
					ret = b2f(file_out, buffer, (pos_end - pos_start + 1)*sizeof(log_unit_t));
					if (ret)
					{
						printf("ERROR: saving error, b2f() return %d!\r\n", ret);
					}
					else
					{
						printf("Config dump saved to file \"%s\".\r\n", file_out);
					}
				}
				else if (format == FORMAT_TEXT)
				{
					FILE *f;
					f = fopen (file_out,"wb");
					if (f == NULL)
					{
						printf("ERROR: saving error, fopen() return NULL!\r\n");
					}
					print_log(f, (plog_unit_t)buffer, (pos_end - pos_start + 1));
					fclose (f);
				}
				else if (format == FORMAT_TOPOS)
				{
					FILE *f;
					f = fopen (file_out,"wb");
					if (f == NULL)
					{
						printf("ERROR: saving error, fopen() return NULL!\r\n");
					}
					print_log_topos(f, (plog_unit_t)buffer, (pos_end - pos_start + 1));
					fclose (f);
				}
				else
				{
					printf("ERROR: saving error, unsupported format!\r\n");
				}
				free(buffer);
			}
		}
		goto close_and_exit;
	case MODE_GET_IMU:
		{
			// Получение обработанных данных. К данным применены все необходимые калибровки, 
			// проведен рассчет и внесены корректировки в соответсвии с конфигурацией.
			imu_data_t imu;
			ret = get_imu_data(&imu);
			if (ret)
			{
				printf("ERROR: get_imu_data() return %d\r\n", ret);
				goto close_and_exit;
			}
			printf("Device orientation:\r\n");
			if ((imu.tilt >  89.5f ||
			     imu.tilt < -89.5f) ||
				(imu.status == 1))
			{
				printf("Azimuth: -\r\n");
			}
			else
			{
				printf("Azimuth: %.1f\r\n", imu.azimuth);
			}
			if (imu.status == 1)
			{
				printf("Tilt:    -\r\n");
			}
			else
			{
				printf("Tilt:    %.1f\r\n", imu.tilt);
			}
		}
		goto close_and_exit;
	case MODE_GET_DIST:
		{
			// Получение дистанции:
			int mm;
			ret = get_distantion(&mm);
			if (ret)
			{
				printf("ERROR: get_distantion() return %d\r\n", ret);
				goto close_and_exit;
			}
			printf("Distantion: %dmm\r\n", mm);
		}
		goto close_and_exit;
	case MODE_SET_CONFIG:
		{
			uint8_t *buffer;
			uint32_t len;
			uint16_t crc;
			device_info_t device_info;
			pconfig_t pconf;

			if (file_in == NULL)
			{
				printf("ERROR: Input file no set!\r\n");
				goto close_and_exit;
			}
			
			ret = f2b(file_in, &buffer, &len);
			if (ret)
			{
				printf("ERROR: File \"%s\" open error!\r\n", file_in);
				goto close_and_exit;
			}
			
			pconf = (pconfig_t)buffer;
			
			if (len != sizeof(config_t) ||
				pconf->tag != 0xEA ||
				pconf->version != 0x01)
			{
				printf("ERROR: Incorrect configuration file!\r\n");
				goto close_and_exit;
			}
			
			// Получаем информацию об устройстве:
			ret = get_info(&device_info);
			if (ret)
			{
				printf("ERROR: get_info() return %d\r\n", ret);
				goto close_and_exit;
			}
			
			// Проверяем совпадение серийного номера:
			if (pconf->sn != device_info.serial_number)
			{
#if (DEVELOPER_BUILD != 0)
				printf("WARNING: Device SN (%08x) and config SN (%08x) didn't match!\r\n", 
					device_info.serial_number, pconf->sn);
#else
				printf("ERROR: Device SN (%08x) and config SN (%08x) didn't match!\r\n", 
					device_info.serial_number, pconf->sn);
				goto close_and_exit;
#endif
			}
			
			// Проверка CRC в образе конфигурации:
			crc = pconf->crc16;
			pconf->crc16 = 0xFFFF;
			pconf->crc16 = crc16(buffer, sizeof(config_t));
			if (pconf->crc16 != crc)
			{
#if (DEVELOPER_BUILD != 0)
				printf("WARNING: Incorrect configuration CRC in file \"%s\"!\r\n", file_in);
#else
				printf("ERROR: Incorrect configuration CRC in file \"%s\"!\r\n", file_in);
				goto close_and_exit;
#endif
			}
			
#if (DEVELOPER_BUILD != 0)
			// Если не правильный SN устройства, обновляем его:
			if (pconf->sn != device_info.serial_number)
			{
				// Устанавливаем нужный SN.
				pconf->sn = device_info.serial_number;
				
				// Пересчет CRC:
				pconf->crc16 = 0xFFFF;
				pconf->crc16 = crc16(buffer, sizeof(config_t));
			}
#endif
			
			// Грузим конфигурацию:
			ret = set_config(pconf);
			if (ret)
			{
				printf("ERROR: set_config() return %d!\r\n", ret);
			}
			else
			{
				printf("Configuration wrote!\r\n");
			}
		}
		goto close_and_exit;
#if (DEVELOPER_BUILD != 0)
	case MODE_CALC_CALIBRATION:
	case MODE_CALC_CALIBRATIONA:
		{
			int k = 0;
			#define MAX_K 32
			#define MAXA_K 32768
			#define MEAN_K 6
			pvector3df_t values_a;
			pvector3df_t values_m;
			calibrate_t cal_a;
			calibrate_t cal_m;
			calibrate9_t cal9_a;
			calibrate9_t cal9_m;
			device_info_t device_info;
			
			if (mode == MODE_CALC_CALIBRATION)
			{
				values_a = (pvector3df_t)malloc(sizeof(vector3df_t)*MAX_K);
				values_m = (pvector3df_t)malloc(sizeof(vector3df_t)*MAX_K);
			}
			else if (mode == MODE_CALC_CALIBRATIONA)
			{
				values_a = (pvector3df_t)malloc(sizeof(vector3df_t)*MAXA_K);
				values_m = (pvector3df_t)malloc(sizeof(vector3df_t)*MAXA_K);
			}
			
			// Получаем информацию об устройстве:
			ret = get_info(&device_info);
			if (ret)
			{
				printf("ERROR: get_info() return %d!\r\n", ret);
				goto close_and_exit;
			}
			
			// Проверяем, что имеем совместимую аппаратную версию:
			if (device_info.hrw_version == 0x00000001)
			{
				// Ok, HW = 1.
			}
#if 0
			else if (device_info.hrw_version == 0x00000002)
			{
				// for future
			}
			else if (device_info.hrw_version == 0x00000003)
			{
				// for future
			}
#endif
			else
			{
				printf("ERROR: Incorrect hardware version (0x%08x)!\r\n", device_info.hrw_version);
				goto close_and_exit;
			}

			if (mode == MODE_CALC_CALIBRATION)
			{
				// Получаем данные для вычисления калибровки:
				printf("Accumulate calibration data. Position device and press any key (Q - for break).\r\n");
				for (k = 0; k < MAX_K; )
				{
					char ch = getch();
					int ret = 0;
					
					if (ch == 'q' || ch == 'Q')
					{
						printf("WARNING: Calibration break!\r\n");
						goto close_and_exit;
					}
					if (ch == 'e' || ch == 'E')
					{
						printf("WARNING: Calibration not full!\r\n");
						break;
					}
					// Получение калибровочного значения:
					ret = calibration_next(&values_a[k], &values_m[k]);
					if (ret == 2)
					{
						printf("WARNING: calibration_next() return %d, please repeate position!\r\n", ret);
						continue;
					}
					if (ret)
					{
						printf("ERROR: calibration_next() return %d!\r\n", ret);
						goto close_and_exit;
					}
					else
					{
						printf("Calibration value save %d/%d\r\n", k+1, MAX_K);
					}
					
					k++;
				}
			}
			else if (mode == MODE_CALC_CALIBRATIONA)
			{
				//
				k = 0;
				while (k < MAXA_K)
				{
					// get_data();
					// save data();
					// new
					k++;
				}
			}
			
			printf("Work calibration_calc():\r\n");
			ret = calibration_calc(values_a, values_m, k, &cal_a, &cal_m);
			if (ret)
			{
				printf("ERROR: calibration_calc() return %d!\r\n", ret);
				goto close_and_exit;
			}
			printf("Work calibration_calc9():\r\n");
			ret = calibration_calc9(values_a, values_m, k, &cal9_a, &cal9_m);
			if (ret)
			{
				printf("ERROR: calibration_calc9() return %d!\r\n", ret);
				goto close_and_exit;
			}
			
#if ( DEVELOPER_BUILD == 0 )
			// Выводим полученные значения на экран:
			printf("Calibrated!\r\n");
			printf("Magnetometer values:\r\n  Offset = { %.4ff, %.4ff, %.4ff }\r\n  Koeff = { %.4ff, %.4ff, %.4ff }\r\n",
				cal_m.offset.x, cal_m.offset.y, cal_m.offset.z, cal_m.koeff.x, cal_m.koeff.y, cal_m.koeff.z);
			printf("Accelerometer values:\r\n  Offset = { %.4ff, %.4ff, %.4ff }\r\n  Koeff = { %.4ff, %.4ff, %.4ff }\r\n",
				cal_a.offset.x, cal_a.offset.y, cal_a.offset.z, cal_a.koeff.x, cal_a.koeff.y, cal_a.koeff.z);
#endif
			// Если задан файл для сохранения, требуется сгенерировать конфигурацию:
			if (file_out)
			{
				config_t conf;
				
				if (format != FORMAT_TEXT &&
					format != FORMAT_BINARY &&
					format != FORMAT_UND)
				{
					printf("ERROR: Unsupported format set!\r\n");
					goto close_and_exit;
				}
				
				printf("Generate configuration...\r\n");
				
				// Очищаем конфигурацию:
				memset((uint8_t*)&conf, 0xFF, sizeof(config_t));
				
				// Заполняем поля:
				// Системные:
				conf.tag     = 0xEA;
				conf.version = 0x01;
				conf.crc16   = 0xFFFF;
				conf.memsize = 8; // 8 * 8kb=64kb
				conf.sn      = device_info.serial_number;
				
				// Калибровочные коэффициенты:
				conf.mag_cal.koeff.x = cal_m.koeff.x;
				conf.mag_cal.koeff.y = cal_m.koeff.y;
				conf.mag_cal.koeff.z = cal_m.koeff.z;
				conf.mag_cal.offset.x = cal_m.offset.x;
				conf.mag_cal.offset.y = cal_m.offset.y;
				conf.mag_cal.offset.z = cal_m.offset.z;
				conf.acc_cal.koeff.x = cal_a.koeff.x;
				conf.acc_cal.koeff.y = cal_a.koeff.y;
				conf.acc_cal.koeff.z = cal_a.koeff.z;
				conf.acc_cal.offset.x = cal_a.offset.x;
				conf.acc_cal.offset.y = cal_a.offset.y;
				conf.acc_cal.offset.z = cal_a.offset.z;
				
				// Смещения нулевые:
				conf.correct_dist  = 0;
				conf.correct_mag   = 0.0f;
				conf.correct_tilt  = 0.0f;
				
				// Перестановка по осям:
				switch (device_info.hrw_version)
				{
				case 0x0000:
					// Голая плата:
					conf.acc_shuff.x = 1;
					conf.acc_shuff.y = 2;
					conf.acc_shuff.z = 3;
					conf.mag_shuff.x = 1;
					conf.mag_shuff.y = 3;
					conf.mag_shuff.z = 2;
					break;
				case 0x00000001:
					// Первая аппаратная версия:
					conf.acc_shuff.x = 2;
					conf.acc_shuff.y = -3;
					conf.acc_shuff.z = -1;
					conf.mag_shuff.x = 3;
					conf.mag_shuff.y = -2;
					conf.mag_shuff.z = -1;
					break;
#if 0 // for future
				case 0x00000002:
					conf.acc_shuff.x = 1;
					conf.acc_shuff.y = 2;
					conf.acc_shuff.z = 3;
					conf.mag_shuff.x = 1;
					conf.mag_shuff.y = 3;
					conf.mag_shuff.z = 2;
					break;
#endif
				default:
					printf("ERROR: Unknown hardware version (unavaible mesh-information)!\r\n");
					break;
				}
				
				// Вычисление углов между G и M векторами:
				conf.mag_tilt = v3f_angle(&values_m[0], &values_a[0]);
				
				// Считаем CRC16 структуры:
				conf.crc16 = crc16((uint8_t*)&conf, sizeof(config_t));
				
				// Формат для сохранения:
				if (format == FORMAT_BINARY ||
					format == FORMAT_UND)
				{
					ret = b2f(file_out, (uint8_t*)&conf, sizeof(config_t));
					if (ret)
					{
						printf("ERROR: Configuration save error!\r\n");
						goto close_and_exit;
					}
					else
					{
						printf("Configuration save!\r\n");
					}
				}
				else if (format == FORMAT_TEXT)
				{
#if 0
					printf("ERROR: Now unsupport TEXT format!\r\n");
					goto close_and_exit;
#else
					FILE *f;
					f = fopen (file_out,"wb");
					if (f == NULL)
					{
						printf("ERROR: saving error, fopen() return NULL!\r\n");
					}
					print_config(f, &conf);
					fclose (f);
#endif
				}
			}
			else
			{
				//NOP
			}
		}
		goto close_and_exit;
#endif
#if (DEVELOPER_BUILD != 0)
	case MODE_CALC_ORIENTATION:
		//while (1) 
		{
			pconfig_t conf;
			
			float azimuth = 0.0f;
			float tilt = 0.0f;
			
			vector3d_t acc = { 0, 0, 0 };
			vector3d_t mag = { 0, 0, 0 };
			
			vector3df_t acc_cal = { 0.0f, 0.0f, 0.0f };
			vector3df_t mag_cal = { 0.0f, 0.0f, 0.0f };
			
			if (file_in)
			{
				uint32_t len;
				uint16_t crc;
				
				// Грузим конфигурацию и берем из нее калибровочные значения:
				ret = f2b(file_in, (uint8_t**)&conf, &len);
				if (ret)
				{
					printf("ERROR: Cannot load config file \"%s\"!\r\n", file_in);
					goto close_and_exit;
				}
				crc = conf->crc16;
				conf->crc16 = 0xFFFF;
				if (len != sizeof(config_t) ||
					conf->tag != 0xEA ||
					conf->version != 0x01 ||
					crc16((uint8_t*)conf, len) != crc)
				{
					printf("ERROR: Incorrect config file \"%s\"!\r\n", file_in);
					goto close_and_exit;
				}
			}
			else
			{
				printf("ERROR: No calibration file!\r\n");
				goto close_and_exit;
			}
			
			ret = get_raw_data(OP_GET_LSM_MAG, &mag);
			if (ret)
			{
				printf("ERROR: get_raw_data() return %d!\r\n", ret);
				goto close_and_exit;
			}
			ret = get_raw_data(OP_GET_LSM_ACC, &acc);
			if (ret)
			{
				printf("ERROR: get_raw_data() return %d!\r\n", ret);
				goto close_and_exit;
			}
			
			// Накладываем калибровку:
			ret = calibration_proc(&acc_cal, &mag_cal, &acc, &mag, conf);
			if (ret)
			{
				printf("ERROR: calibration_proc() return %d!\r\n", ret);
				goto close_and_exit;
			}
			
			printf("acc = { %.3f, %.3f, %.3f }, mod(acc) = %.3f;\r\nmag = { %.3f, %.3f, %.3f }, mod(mag) = %.3f\r\n\r\n",
				acc_cal.x, acc_cal.y, acc_cal.z, v3f_mod(&acc_cal), 
				mag_cal.x, mag_cal.y, mag_cal.z, v3f_mod(&mag_cal));
			
			// Не обязаетельно, нормируем вектора:
			v3f_norm(&acc_cal);
			v3f_norm(&mag_cal);
			
			// Вычисляем углы:
			ret = calc_angles(&acc_cal, &mag_cal, &tilt, &azimuth);
			
			// Выводим информацию:
			printf("Orientation:\r\n");
				printf("\ttilt = %3.1f;\r\n", tilt);
			if (ret == 0)
				printf("\tazimuth = %3.1f\r\n", azimuth);
			else
				printf("\tazimuth = -\r\n");
			
			free(conf);
		}
		goto close_and_exit;
#endif
#if (DEVELOPER_BUILD != 0)
	case MODE_CALC_FRW:
		{
			int ret;
			uint32_t len;
			uint8_t *data;
			uint8_t key[32] = FRW_CRYPT_KEY;
			pimg_t pimg;
			pimg_bin_t pimg_bin;
			
			if (file_in == NULL)
			{
				printf("ERROR: No input file!\r\n");
				goto close_and_exit;
			}
			if (file_out == NULL)
			{
				printf("ERROR: No output file!\r\n");
				goto close_and_exit;
			}
			
			ret = f2b(file_in, &data, &len);
			if (ret)
			{
				printf("ERROR: Cannot open file \"%s\"!\r\n", file_in);
				goto close_and_exit;
			}
			printf("File \"%s\" loaded!\r\n", file_in);
			
			// Проверка размера образа:
			if (len <= IMG_SIZE)
			{
				if (len & 0x3F)
					len = (len & ~0x3F) + 0x40;
				
				pimg = (pimg_t)malloc(sizeof(img_t));
				pimg_bin = (pimg_bin_t)pimg->data;
				
				printf("Allocate %d bytes for img.\r\n", (int)sizeof(img_t));
				memset(pimg->data, 0xFF, IMG_SIZE);
				memcpy(pimg->data, data, len);
				free(data);
			}
			else
			{
				printf("ERROR: Firmware image too big!\r\n");
				goto close_and_exit;
			}
			
			// Проверяем заголовок в образе, чтобы не создать файл обновления из случайного файла:
			if (pimg_bin->tag != 0xAB6E37F4)
			{
				printf("ERROR: Incorrect vector in firmware image!\r\n");
				goto close_and_exit;
			}
			// Устанавливаем кооректный размер:
			pimg_bin->size = len-0x100;
			// Считаем контрольную сумму загружаемого файла:
			pimg_bin->crc32 = crc32(CRC32_FRW_START, pimg_bin->data, pimg_bin->size);
			
			// Заполнение не шифруемого заголовка
			pimg->tag    = 0x73EB8A3B;
			pimg->size   = len;
			pimg->hw_ver = pimg_bin->hw_ver; // зарезервированный байт в образе для версии апп. платформы.
			pimg->res[0] = 0x00000000;
			pimg->res[1] = 0x00000000;
			pimg->res[2] = 0x00000000;
			pimg->res[3] = 0x00000000;
			printf("Header created!\r\n");
			
			// Шифруем тело:
			printf("Crypt body... ");
			gost_init((uint32_t*)key);
			gost_crypt_cbc((uint32_t*)pimg->data, len>>2, GOST_CBC_VECT_1, GOST_CBC_VECT_2);
			printf("OK!\r\n");
			
			// CRC32 сохраняется для зашифрованных данных:
			printf("Calculate CRC32... ");
			pimg->crc32  = crc32(0x00000000, pimg->data, len);
			printf("OK!\r\n");
			// Данное CRC предназначается только для проверки CRC на уровне пользователя,
			// внутрь устройства оно не попадает.
			
			// Сохранение полученного файла:
			ret = b2f(file_out, (uint8_t*)pimg, sizeof(img_t)-0x7000+len);
			if (ret)
			{
				printf("ERROR: Cannot save file \"%s\"!\r\n", file_out);
				goto close_and_exit;
			}
			else
			{
				printf("New firmware update created in file \"%s\"!\r\n", file_out);
			}
			
			// Освобождаем память:
			free(pimg);
		}
		goto exit;
#endif
	case MODE_GRAPH_COMPASS:
#if (GRAPH_SUPPORT != 0)
		{
			// Вход в графический режим:
			ret = graph_into(&device);
			if (ret)
			{
				printf("ERROR: Graph mode error (%d)!\r\n", ret);
			}
		}
		goto close_and_exit;
#else
		printf("ERROR: Graph mode unsupported!\r\n");
		goto close_and_exit;
#endif
	case MODE_GRAPH_CONFIG:
#if (GRAPH_SUPPORT != 0)
		{
			HINSTANCE hInst = NULL;
			hInst = GetModuleHandle(NULL);
			if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DLG1), HWND_DESKTOP, (DLGPROC)ConfigDialogProc))
			{
				printf("WARNING: DialogBox() returned not NULL.\r\n");
			}
			else 
			{
				printf("WARNING: DialogBox() returned NULL.\r\n");
			}
		}
		goto close_and_exit;
#else
		printf("ERROR: Graph mode unsupported!\r\n");
		goto close_and_exit;
#endif
#if (DEVELOPER_BUILD != 0)
	case MODE_LOADER:
		if (id == -1)
		{
			printf("ERROR: Serial number didn't set!\r\n");
		}
		else if (file_out != NULL)
		{
write_loader:
			// Модификация образа.
			// 1. Устанавливаем вектор защиты микропрограммы:
			((uint32_t*)loader_bin)[0x2FC/4] = 0x87654321; // protect vector
			// 2. Устанавливаем серийный номер:
			((uint32_t*)loader_bin)[0x34/4]  = id; // serial number
			
			ret = b2f(file_out, loader_bin, loader_len);
			if (ret)
			{
				printf("ERROR: saving error, b2f() return %d!\r\n", ret);
			}
			else
			{
				printf("Loader binary saved to file \"%s\".\r\n", file_out);
			}
			
			if (disk != ' ')
			{
				char cmd[100];
				// Буква диска задана... копируем образ на него.
				// Копирование 
				
				printf("Copy image to disk %c:\r\n", disk);
				
				// 1. Удаляем текущую микропрограмму:
				sprintf(cmd, "del /P /F %c:\\firmware.bin", disk);
				system(cmd);
				// 2. Копируем новый образ:
				sprintf(cmd, "copy /-Y %s %c:\\firmware.bin", file_out, disk);
				system(cmd);
			}
		}
		else
		{
			file_out = "temp.bin";
			printf("WARNING: File out no set, binary saved as %s!\r\n", file_out);
			goto write_loader;
		}
		goto exit;
#endif
#if (DEVELOPER_BUILD != 0)
	case MODE_LASER_ON:
		laser_on();
		goto close_and_exit;
	case MODE_LASER_OFF:
		laser_off();
		goto close_and_exit;
	case MODE_LASER_EN:
		laser_en();
		goto close_and_exit;
	case MODE_TEST_LCD:
		{
			uint8_t frame[47];
#if 1
			if (pos_start != -1)
			{
				// Если заданы границы, требуется обработать вывод на экран значения, 
				// используется для составления карты сегментов.
				if (pos_end < pos_start)
					pos_end = pos_start; 
				for (i = pos_start; i < pos_end+1; i++)
					lcd_bit_access(i, 1);
			}
			else
			{
				// Иначе простой тестовый вывод.
				
				// Служебные цифры:
				lcd_arr(lcd_arr_batt, 1);
				lcd_arr(lcd_arr_las, 1);
				lcd_arr(lcd_unit, 1);
				//lcd_arr(lcd_setup, 1);
				//lcd_arr(lcd_cal, 1);
				//lcd_arr(lcd_of_32, 1);
				// Измерения:
				put_tilt(-237);
				put_azimuth(3442);
				put_dist(12054);
			}
			
			// Получаем буфер кадра:
			get_frame(frame, sizeof(frame));
			// Отправляем:
			laser_lcd_send(frame, sizeof(frame));
			laser_lcd_send(frame, sizeof(frame));
			laser_lcd_send(frame, sizeof(frame));
			laser_lcd_send(frame, sizeof(frame));
#elif 0
			memset(frame, 0x00, 47);
			laser_lcd_send(frame, sizeof(frame));
			laser_lcd_send(frame, sizeof(frame));
			laser_lcd_send(frame, sizeof(frame));
			laser_lcd_send(frame, sizeof(frame));
			laser_lcd_send(frame, sizeof(frame));
			laser_lcd_send(frame, sizeof(frame));
			laser_lcd_send(frame, sizeof(frame));
#else
			laser_lcd_send_low();
#endif
		}
		goto close_and_exit;
	case MODE_READ_ISP:
		// Режим -заточен на чтение ISP-области данных:
		{
			#define READ_ADDR 0x1FFF0000
			#define READ_SIZE 0x4000
			uint8_t *data = (uint8_t*)malloc(READ_SIZE*sizeof(uint8_t));
			FILE *f;
			printf("ISP dump...\r\n");
			ret = frw_read(data, READ_ADDR, READ_SIZE);
			if (ret) {
				printf("frw_read() return %d!\r\n", ret);
			} else {
				printf("ISP dump ok! Saving...\r\n");
				
				f = fopen("dump_isp.bin", "wb");
				fwrite(data, 1, READ_SIZE, f);
				fclose(f);
			}
		}
		goto close_and_exit;
#endif
	case MODE_UND:
		printf("ERROR: Undefined utility mode!\r\n");
		goto exit;
	default:
		printf("ERROR: Unknown error!\r\n");
		goto close_and_exit;
	}

close_and_exit:
	hid_close(&device);
exit:
	return 0;
}

/*
	help()
 */
void help(void)
{
	printf("lascon%s.exe - main work utility v%d.%d (build: %s %s)\r\n", DEVELOPER_BUILD?"d":"", VERSION, SUBVERSION, __DATE__, __TIME__);
	printf("Developed by Igor Kovalenko (vk.com/igorkov or fsp@igorkov.org)\r\n\r\n");
	printf("Supported arguments:\r\n");
	printf("\t-help\r\n");
	printf("\t\tPrint this help.\r\n");
	printf("\t-get info\r\n");
	printf("\t-info\r\n");
	printf("\t\tGet device info (SN, Version, memory info).\r\n");
	printf("\t-get config\r\n");
	printf("\t\tRead configuration (save in binary or text).\r\n");
	printf("\t-get log\r\n");
	printf("\t\tRead log file. Need -start and -end setup.\r\n");
	printf("\t-get imu\r\n");
	printf("\t\tOrientation calculate.\r\n");
	printf("\t-get dist\r\n");
	printf("\t\tDistance calculate.\r\n");
	printf("\t-set config\r\n");
	printf("\t\tWrite configuration (only binary format).\r\n");
	printf("\t-set firmware\r\n");
	printf("\t\tUpload new firmware.\r\n");
	printf("\t-start N\r\n");
	printf("\t\tSetup start position.\r\n");
	printf("\t-end N\r\n");
	printf("\t\tSetup end position.\r\n");
	printf("\t-erase config\r\n");
	printf("\t\tErase configuration.\r\n");
	printf("\t-erase log\r\n");
	printf("\t\tErase log file from device.\r\n");
	printf("\t-format text/binary/csv/topos\r\n");
	printf("\t\tChange output format.\r\n");
	printf("\t-out filename\r\n");
	printf("\t\tSet output file name.\r\n");
	printf("\t-in filename\r\n");
	printf("\t\tSet input file name.\r\n");
#if (BT_SUPPORT != 0)
	printf("\t-bt PORT_NUMBER\r\n");
	printf("\t\tUse bluetooth interface.\r\n");
#endif
#if (GRAPH_SUPPORT != 0)
	// Графические варианты работы программы:
	printf("\t-graph compass\r\n");
	printf("\t\tGraphical mode. Create window with compass.\r\n");
	printf("\t-graph config\r\n");
	printf("\t\tGraphical mode. Open configuration dialog.\r\n");
	// В дальнейшем данные режимы работы лучше будет вынести в отдельные утилиты.
#endif
#if (DEVELOPER_BUILD != 0)
	printf("\r\nDeveloper supported arguments:\r\n");
	printf("\t-get raw\r\n");
	printf("\t\tRead sensors data in RAW-format.\r\n");
	printf("\t-get memory\r\n");
	printf("\t\tRead data from EEPROM (direct access).\r\n");
	printf("\t-get temp\r\n");
	printf("\t\tRead temperature sensors data.\r\n");
	printf("\t-set memory\r\n");
	printf("\t\tWrite data to EEPROM (direct access).\r\n");
	printf("\t-frw erase (or -erase firmware)\r\n");
	printf("\t\tErase MCU firmware.\r\n");
	printf("\t-frw kill_loader (or -erase loader)\r\n");
	printf("\t\tErase MCU loader firmware.\r\n");
	printf("\t-frw generate\r\n");
	printf("\t\tGenerate update file from binary.\r\n");
	printf("\t-frw update (or -set firmware)\r\n");
	printf("\t\tUpload new firmware.\r\n");
	printf("\t-frw test\r\n");
	printf("\t\tTest FRW logic (GOST algorithm, FRW image).\r\n");
	printf("\t-calc orientation\r\n");
	printf("\t\tDebug functions for calculate orientation.\r\n");
	printf("\t-calc calibration\r\n");
	printf("\t\tDebug functions for PC-calibration (optional generate config).\r\n");
	printf("\r\nLoader functions:\r\n");
	printf("\t-loader\r\n");
	printf("\t\tGenerate loader file (with ID and copy to disk).\r\n");
	printf("\t-id ID\r\n");
	printf("\t\tSetup loader id.\r\n");
	printf("\t-disk X\r\n");
	printf("\t\tDisk for copy.\r\n");
	printf("Sample: lascond.exe -loader -id 00000001h -disk h\r\n");
	printf("\r\nLaser functions:\r\n");
	printf("\t-laser on\r\n");
	printf("\t\tTurn on laser device.\r\n");
	printf("\t-laser off\r\n");
	printf("\t\tTurn off laser device.\r\n");
	printf("\t-laser on\r\n");
	printf("\t\tLaser pointer on.\r\n");
	printf("\t-laser lcd [-start N [-end M]]\r\n");
	printf("\t\tTurn on LCD and send it data. Optional testing segments.\r\n");
	//printf("\t-xxx");
	//pritnf("\t\tyyy");
#endif
#if (DEVELOPER_BUILD != 0)
	printf("Debug read memory:\r\n");
	printf("\t-read\r\n");
#endif
	printf("\r\n");
	//exit(0);
}

/*
	print_config()
	
	Генерация текстовой версии файла конфигурации.
 */
void print_config(FILE *f, pconfig_t conf)
{
/*
	config.txt
	VERSION: 0x01
	# Серийный номер устройства, с которого была получена конфигурация:
	SN: 0x1234
	# Угол вектора магнитного поля к горизонту:
	MAG_VECT: 0.67
	#    Kx    Ky    Kz    Ox      Oy    Oz
	MAG: 1.027 1.012 0.992 0.014   0.026 -0.043
	ACC: 1.033 0.925 1.012 -0.0223 0.002 0.023
	# Перестановка осей:
	MAG: 1 3 2
	ACC: 1 2 3
	# Угол магнитного поля:
	MAG TILT: 65.0
 */
	
	fprintf(f, "VERSION: %d\r\n", conf->version);
	fprintf(f, "# Серийный номер устройства, с которого была получена конфигурация:\r\n");
	fprintf(f, "SN: %08x\r\n", conf->sn);
	//fprintf(f, "# Угол вектора магнитного поля к горизонту:\r\n");
	//fprintf(f, "MAG_VECT: %3.1f\r\n", conf->mag_tilt);
	fprintf(f, "#    Kx    Ky    Kz    Ox    Oy    Oz\r\n");
	fprintf(f, "MAG: %1.3f %1.3f %1.3f %1.3f %1.3f %1.3f\r\n", 
		conf->mag_cal.koeff.x, conf->mag_cal.koeff.y, conf->mag_cal.koeff.z, 
		conf->mag_cal.offset.x, conf->mag_cal.offset.y, conf->mag_cal.offset.z);
	fprintf(f, "ACC: %1.3f %1.3f %1.3f %1.3f %1.3f %1.3f\r\n", 
		conf->acc_cal.koeff.x, conf->acc_cal.koeff.y, conf->acc_cal.koeff.z, 
		conf->acc_cal.offset.x, conf->acc_cal.offset.y, conf->acc_cal.offset.z);
	fprintf(f, "# Перестановка осей:\r\n");
	fprintf(f, "MAG: %d %d %d\r\n", 
		conf->mag_shuff.x, conf->mag_shuff.y, conf->mag_shuff.z);
	fprintf(f, "ACC: %d %d %d\r\n",
		conf->acc_shuff.x, conf->acc_shuff.y, conf->acc_shuff.z);
	fprintf(f, "# Угол магнитного поля:\r\n");
	fprintf(f, "MAG_TILT: %3.1f\r\n", conf->mag_tilt);
	fprintf(f, "# Магнитное склонение:\r\n");
	fprintf(f, "MAG_DECL: %3.1f\r\n", conf->correct_mag);
	fprintf(f, "# Корректнировка угла к горизонту:\r\n");
	fprintf(f, "TILT_CORRECTION: %3.1f\r\n", conf->correct_tilt);
	fprintf(f, "\r\n");
	
	return;
}

#define FLAG_VECTOR 0x01

/*
	print_log()
	
	Генерация пикетажного файла в текстовом формате (собственный формат).
 */
void print_log(FILE *f, plog_unit_t log, int n)
{
/*
	log.txt
	
	# ID     SUB ID    DIST (mm)   AZIMUT (deg)   ANGLE (deg)    FAGS
	00001    -         10234       345.3          10.2           T
	00002    -         3452        145.1          -15.2          T
	00003    -         5643        245.7          -80.5          T
	00004    -         9372        275.5          22.2           T
	00004    00001     1272        175.5          3.2            VE
	00004    00002     2172        275.5          5.2            V
	00004    00003     3372        15.5           86.2           V
	00005    -         16234       332.9          13.2           T

	# T - thread (нитка хода)
	# V - vector (боковой вектор, съемка стен)
	# E - error (данные могут быть ошибочными)
*/

	int i;
	int lastid = 0;
	if (f == NULL)
	{
		f = stdout;
	}
	
	fprintf(f,     "# ID     SUB ID    DIST (mm)   AZIMUTH (deg)   ANGLE (deg)    FAGS\r\n");
	for (i = 0; i < n; i++)
	{
		uint8_t crc = log[i].crc8;
		log[i].crc8 = 0xFF;
		
		if ((log[i].type & FLAG_VECTOR) == 0)
			lastid = log[i].id;
		//         "00001    -         10234       345.3          10.2           T
		fprintf(f, "%05x      ", lastid);
		if (log[i].type & FLAG_VECTOR)
			fprintf(f, "%05x      ", ABS(log[i].id));
		else
			fprintf(f, "-         ");
		fprintf(f, "%5d         ", log[i].dist);
		fprintf(f, "%3.1f           ", (float)log[i].azimuth/10);
		fprintf(f, "%3.1f          ", (float)log[i].tilt/10);
		fprintf(f, "%s", (log[i].type & FLAG_VECTOR)?"V":"T");
		fprintf(f, "%s", (crc != crc8((uint8_t*)&log[i], sizeof(log_unit_t)))?"E":"");
		fprintf(f, "\r\n");
		
		fprintf(f, "# T - thread (нитка хода)\r\n");
		fprintf(f, "# V - vector (боковой вектор, съемка стен)\r\n");
		fprintf(f, "# E - error (данные могут быть ошибочными)\r\n");
		fprintf(f, "\r\n");
	}
	return;
}

/*
	print_log_topos()
	
	Генерация пикетажного файла в формате Topos.
 */
void print_log_topos(FILE *f, plog_unit_t log, int n)
{
/*
	log.dat
	
	#cave Название пещеры / хода
	%Комментарии к топосъемке

	#from_to												
	#data_order L Az An
						
	#survey ^ug Авторы топосъемки
	#survey_title Название участка
	#survey_date xx.xx.20xx

	%№    L,м   Аз,°     угол,°  л п в н Комментарии	
	1     2     2   30    0      - - - - % 
	2     3     1   0    -20     - - - - % 
	3     4     1.5 65   +60     - - - - % 
	4     5     6   0    +90     - - - - % 

	#end_survey
*/

	int i;
	int lastid = 0;
	if (f == NULL)
	{
		f = stdout;
	}
	
	fprintf(f,     "#cave Название пещеры / хода\r\n");
	fprintf(f,     "%%Комментарии к топосъемке\r\n");
	fprintf(f,     "\r\n");
	fprintf(f,     "#from_to\r\n");
	fprintf(f,     "#data_order L Az An   l r u d\r\n");
	fprintf(f,     "\n");
	fprintf(f,     "#survey ^ug Авторы топосъемки\r\n");
	fprintf(f,     "#survey_title Название участка\r\n");
	fprintf(f,     "#survey_date %s\r\n", __DATE__);
	fprintf(f,     "\r\n");
	fprintf(f,     "\r\n");
	fprintf(f,     "%% A   B    L,м      Аз,°    угол,°    л    п    в    н    Комментарии\r\n");
	for (i = 0; i < n; i++)
	{
		uint8_t crc = log[i].crc8;
		log[i].crc8 = 0xFF;
		
		if ((log[i].type & FLAG_VECTOR) == 0)
			lastid = log[i].id;
		else
			continue;
		fprintf(f, "%05x       ", lastid);
		fprintf(f, "%3.1f      ", (float)log[i].dist);
		fprintf(f, "%3.1f      ", (float)log[i].azimuth/10);
		fprintf(f, "%3.1f      ", (float)log[i].tilt/10);
		fprintf(f, "- ");
		fprintf(f, "- ");
		fprintf(f, "- ");
		fprintf(f, "- ");
		fprintf(f, "%% %s", (crc != crc8((uint8_t*)&log[i], sizeof(log_unit_t)))?"CRC Error!":"");
		fprintf(f, "\r\n");
	}
	fprintf(f,     "#end_survey\r\n");
	return;
}

/*
	printf_buffer()
	
	Вывод на экран буфера в виде HEX-таблицы.
 */
void printf_buffer(FILE *f, uint16_t addr, uint8_t *data, int len)
{
	int i;
	for (i=0; i < len; i++)
	{
		if ((i % 16) == 0)
			fprintf(f, "%04x: ", addr+i);
		fprintf(f, "%02x ", data[i]);
		if ((i % 16) == 15)
			fprintf(f, "\r\n");
	}
	if (i%16) fprintf(f, "\r\n");
}

#if (GRAPH_SUPPORT != 0)
BOOL CALLBACK ConfigDialogProc(HWND hwndDlg, 
						UINT message, 
						WPARAM wParam, 
						LPARAM lParam) 
{
	static pconfig_t pconf = (pconfig_t)NULL;
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
	HWND hwnd = hwndDlg;    // owner window
	//HANDLE hf;              // file handle
	uint32_t size;

#if (DEVELOPER_BUILD == 1)
	//printf("ConfigDialogProc()... hwndDlg = 0x%08x, message = %d\n", hwndDlg, message);
#endif
	
	switch (message) 
	{
	case WM_PAINT:
		break;
	
	case WM_SETFONT:
		break;
	
	case WM_INITDIALOG:
#if (DEVELOPER_BUILD == 1)
		printf("hwndDlg = %08x\n", (unsigned int)hwndDlg);
#endif

#if (DEVELOPER_BUILD != 1)
		// Пользовательская сборка "не умеет" менять расположение осей:
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_ACC_MESHX), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_ACC_MESHY), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_ACC_MESHZ), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_MAG_MESHX), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_MAG_MESHY), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_MAG_MESHZ), FALSE);
		// Так же не умеет менять "системные" поля структуры (ID, SerialNumber, MemorySize, etc.):
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_CNFVER), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_HWVER), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_SN), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_MEMSIZE), FALSE);
#endif
		// Отключаем кнопки в первый момент:
		EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SAVE_CONFIG), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SAVE_FILE), FALSE);
		// Пока не имеем возможности калибровать:
		EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_CALIBRATE), FALSE);
		
		// Ecли устройство не подключено, требуется деактивировать кнопки работы с ним:
		if (device.hDeviceHandle == NULL)
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SAVE_CONFIG), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_LOAD_CONFIG), FALSE);
		}
		
		// Инициализация полей:
		//SetDlgItemText(hwndDlg, IDC_EDT_ACC_MESHX, (LPCTSTR)"-1");
		//GetDlgItemText(hwndDlg, IDC_EDT_ACC_MESHX, value);
		
		//SendMessage(GetDlgItem(hwndDlgForm1, IDC_TRB1), TBM_SETRANGE, TRUE, 100);
		//SendMessage(GetDlgItem(hwndDlgForm1, IDC_TRB1), TBM_SETRANGEMIN, TRUE, 0);
		//SendMessage(GetDlgItem(hwndDlgForm1, IDC_TRB1), TBM_SETRANGEMAX, TRUE, 100);
		break;

	case WM_COMMAND:
		// 1. Первый этап:
		switch (LOWORD(wParam)) 
		{
		case IDC_BTN_EXIT:
			// Обработка кнопки выхода:
			EndDialog(hwndDlg, wParam);
			break;
		case IDC_BTN_SAVE_CONFIG:
		case IDC_BTN_LOAD_CONFIG:
			break;
		case IDC_BTN_SAVE_FILE:
		case IDC_BTN_LOAD_FILE:
			{
				// Initialize OPENFILENAME
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hwnd;
				ofn.lpstrFile = szFile;
				// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
				// use the contents of szFile to initialize itself.
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = "All\0*.*\0Laser Configuration\0*.LASCFG\0Binary\0*.BIN\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				
				// Display the Open dialog box. 
				if (GetOpenFileName(&ofn)==TRUE) 
				{
					// ok
				}
				else
				{
					return FALSE;
				}
					//hf = CreateFile(ofn.lpstrFile, 
					//				GENERIC_READ,
					//				0,
					//				(LPSECURITY_ATTRIBUTES) NULL,
					//				OPEN_EXISTING,
					//				FILE_ATTRIBUTE_NORMAL,
					//				(HANDLE) NULL);
			}
		}
		// 2. Второй этап:
		switch (LOWORD(wParam)) 
		{
		case IDC_BTN_LOAD_CONFIG:
		case IDC_BTN_LOAD_FILE:
			{
				char abFieldData[128];
				// 1. Чтение конфигурации:
				if (LOWORD(wParam) == IDC_BTN_LOAD_FILE)
				{
					if (pconf != NULL)
						free(pconf);
					f2b((char*)ofn.lpstrFile, (uint8_t**)&pconf, &size);
				}
				else if (LOWORD(wParam) == IDC_BTN_LOAD_CONFIG)
				{
					if (pconf == NULL)
						pconf = (pconfig_t)malloc(sizeof(config_t));
					get_config(pconf);
				}
				
				// 2. Проверка конфигурации:
				if (size != sizeof(config_t))
				{
					//...
				}
				//...
				//...
				
				// 3. Записываем параметры конфига в поля диалога:
				sprintf(abFieldData, "%08d", pconf->version);
				SetDlgItemText(hwndDlg, IDC_EDT_CNFVER, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%08d", pconf->version);
				SetDlgItemText(hwndDlg, IDC_EDT_HWVER, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "0x%08X", pconf->sn);
				SetDlgItemText(hwndDlg, IDC_EDT_SN, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%dkb", pconf->memsize*8);
				SetDlgItemText(hwndDlg, IDC_EDT_MEMSIZE, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->acc_cal.koeff.x);
				SetDlgItemText(hwndDlg, IDC_EDT_ACC_KX, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->acc_cal.koeff.y);
				SetDlgItemText(hwndDlg, IDC_EDT_ACC_KY, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->acc_cal.koeff.z);
				SetDlgItemText(hwndDlg, IDC_EDT_ACC_KZ, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->acc_cal.offset.x);
				SetDlgItemText(hwndDlg, IDC_EDT_ACC_OX, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->acc_cal.offset.y);
				SetDlgItemText(hwndDlg, IDC_EDT_ACC_OY, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->acc_cal.offset.z);
				SetDlgItemText(hwndDlg, IDC_EDT_ACC_OZ, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->mag_cal.koeff.x);
				SetDlgItemText(hwndDlg, IDC_EDT_MAG_KX, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->mag_cal.koeff.y);
				SetDlgItemText(hwndDlg, IDC_EDT_MAG_KY, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->mag_cal.koeff.z);
				SetDlgItemText(hwndDlg, IDC_EDT_MAG_KZ, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->mag_cal.offset.x);
				SetDlgItemText(hwndDlg, IDC_EDT_MAG_OX, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->mag_cal.offset.y);
				SetDlgItemText(hwndDlg, IDC_EDT_MAG_OY, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.4f", pconf->mag_cal.offset.z);
				SetDlgItemText(hwndDlg, IDC_EDT_MAG_OZ, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%d", pconf->acc_shuff.x);
				SetDlgItemText(hwndDlg, IDC_EDT_ACC_MESHX, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%d", pconf->acc_shuff.y);
				SetDlgItemText(hwndDlg, IDC_EDT_ACC_MESHY, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%d", pconf->acc_shuff.z);
				SetDlgItemText(hwndDlg, IDC_EDT_ACC_MESHZ, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%d", pconf->mag_shuff.x);
				SetDlgItemText(hwndDlg, IDC_EDT_MAG_MESHX, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%d", pconf->mag_shuff.y);
				SetDlgItemText(hwndDlg, IDC_EDT_MAG_MESHY, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%d", pconf->mag_shuff.z);
				SetDlgItemText(hwndDlg, IDC_EDT_MAG_MESHZ, (LPCTSTR)abFieldData);
				
				sprintf(abFieldData, "%.2f", pconf->correct_mag);
				SetDlgItemText(hwndDlg, IDC_EDT_MAG_DECL, (LPCTSTR)abFieldData);
				sprintf(abFieldData, "%.2f", pconf->correct_tilt);
				SetDlgItemText(hwndDlg, IDC_EDT_TILT_COMP, (LPCTSTR)abFieldData);
				
				//sprintf(abFieldData, "%.4f", pconf->mag_shuff.x);
				//SetDlgItemText(hwndDlg, IDC_EDT, (LPCTSTR)abFieldData);
				
				// Активируем кнопки сохранения:
				// Сохранение в файл:
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SAVE_FILE), TRUE);
				// Сохранение в устройство только в случае, если устройство подключено:
				if (device.hDeviceHandle != NULL)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SAVE_CONFIG), TRUE);
					// Без устройства не имеем возможности калибровать:
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_CALIBRATE), TRUE);
				}
				break;
			}
		case IDC_BTN_SAVE_CONFIG:
		case IDC_BTN_SAVE_FILE:
			{
				char abFieldData[128];
				int ivalue;
				
				// 1. Чтение всех полей граф. окна в структуру:
				GetDlgItemText(hwndDlg, IDC_EDT_CNFVER, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%d", &ivalue);
				pconf->version = ivalue;
				GetDlgItemText(hwndDlg, IDC_EDT_HWVER, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%d", &ivalue);
				pconf->version = ivalue;
				GetDlgItemText(hwndDlg, IDC_EDT_SN, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "0x%X", &ivalue);
				pconf->sn = ivalue;
				GetDlgItemText(hwndDlg, IDC_EDT_MEMSIZE, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%dkb", &ivalue);
				pconf->memsize = ivalue / 8;
				
				GetDlgItemText(hwndDlg, IDC_EDT_ACC_KX, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->acc_cal.koeff.x);
				GetDlgItemText(hwndDlg, IDC_EDT_ACC_KY, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->acc_cal.koeff.y);
				GetDlgItemText(hwndDlg, IDC_EDT_ACC_KZ, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->acc_cal.koeff.z);
				GetDlgItemText(hwndDlg, IDC_EDT_ACC_OX, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->acc_cal.offset.x);
				GetDlgItemText(hwndDlg, IDC_EDT_ACC_OY, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->acc_cal.offset.y);
				GetDlgItemText(hwndDlg, IDC_EDT_ACC_OZ, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->acc_cal.offset.z);
				GetDlgItemText(hwndDlg, IDC_EDT_MAG_KX, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->mag_cal.koeff.x);
				GetDlgItemText(hwndDlg, IDC_EDT_MAG_KY, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->mag_cal.koeff.y);
				GetDlgItemText(hwndDlg, IDC_EDT_MAG_KZ, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->mag_cal.koeff.z);
				GetDlgItemText(hwndDlg, IDC_EDT_MAG_OX, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->mag_cal.offset.x);
				GetDlgItemText(hwndDlg, IDC_EDT_MAG_OY, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->mag_cal.offset.y);
				GetDlgItemText(hwndDlg, IDC_EDT_MAG_OZ, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->mag_cal.offset.z);
				GetDlgItemText(hwndDlg, IDC_EDT_ACC_MESHX, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%d", &ivalue);
				pconf->acc_shuff.x = ivalue;
				GetDlgItemText(hwndDlg, IDC_EDT_ACC_MESHY, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%d", &ivalue);
				pconf->acc_shuff.y = ivalue;
				GetDlgItemText(hwndDlg, IDC_EDT_ACC_MESHZ, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%d", &ivalue);
				pconf->acc_shuff.z = ivalue;
				GetDlgItemText(hwndDlg, IDC_EDT_MAG_MESHX, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%d", &ivalue);
				pconf->mag_shuff.x = ivalue;
				GetDlgItemText(hwndDlg, IDC_EDT_MAG_MESHY, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%d", &ivalue);
				pconf->mag_shuff.y = ivalue;
				GetDlgItemText(hwndDlg, IDC_EDT_MAG_MESHZ, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%d", &ivalue);
				pconf->mag_shuff.z = ivalue;
				
				GetDlgItemText(hwndDlg, IDC_EDT_MAG_DECL, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->correct_mag);
				GetDlgItemText(hwndDlg, IDC_EDT_TILT_COMP, (LPTSTR)abFieldData, sizeof(abFieldData));
				sscanf(abFieldData, "%f", &pconf->correct_tilt);
				
				// 3. Запись флагов корректности:
				
				// 4. Модификации CRC-конфигурации:
				//...
				
				// 5. Сохранение конфигурации
				if (LOWORD(wParam) == IDC_BTN_SAVE_FILE)
				{
					// в файл:
					b2f((char*)ofn.lpstrFile, (uint8_t*)pconf, sizeof(config_t));
				}
				else if (LOWORD(wParam) == IDC_BTN_SAVE_CONFIG)
				{
					// в устройство:
					set_config(pconf);
				}
			}
			break;
		default:
			break;
		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:
			break;
		case VK_RIGHT:
			break;
		case VK_UP:
			break;
		case VK_DOWN:
			break;
		case VK_HOME:
			break;
		case VK_END:
			break;
		case VK_INSERT:
			break;
		case VK_DELETE:
			break;
		case VK_F2:
			break;
		case VK_ESCAPE:
			printf("VK_ESCAPE!!!\r\n");
			EndDialog(hwndDlg, wParam);
			return TRUE;
		default:
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, wParam); 
		return TRUE; 

	case WM_DESTROY:
		EndDialog(hwndDlg, wParam); 
		return TRUE; 

	default:
		break;
	} 
	return FALSE; 
} 
#endif

#if (GRAPH_SUPPORT != 0)
int graph_into(hid_context *device)
{
	MSG lpMsg;
	WNDCLASS wc;
	RECT Rect;
	HANDLE hInstance;
	DWORD dwWindowStyle = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME);
	int iWindowSizeX, iWindowSizeY;
	
	DWORD dwThreadId;
	
	printf("graph_into()\r\n");

	frame_buffer = (uint32_t*)malloc(sizeof(DWORD) * SIZE_X * SIZE_Y);
	
	hInstance = GetModuleHandle(NULL);

	printf("CreateThread(1) ");

	hThread1 = CreateThread( 
			NULL,                   // default security attributes
			0,                      // use default stack size  
			ProcGenerateFrame,      // thread function name
			NULL,                   // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);           // returns the thread identifier 
	if (hThread1 == NULL)
	{
		ExitProcess(3);
	}

	printf("OK!\r\n");
	
	printf("CreateThread(2) ");

	hThread2 = CreateThread( 
			NULL,                   // default security attributes
			0,                      // use default stack size  
			ProcGetData,            // thread function name
			NULL,                   // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);           // returns the thread identifier 
	if (hThread2 == NULL)
	{
		ExitProcess(3);
	}

	printf("OK!\r\n");
	
	// Заполняем структуру класса окна
	wc.style         = (CS_HREDRAW | CS_VREDRAW);
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szClassName;

	Rect.left = 100;
	Rect.right = Rect.left + SIZE_X;
	Rect.top = 100;
	Rect.bottom = Rect.top + SIZE_Y;
	AdjustWindowRect(&Rect, dwWindowStyle, FALSE);
	iWindowSizeX = Rect.right - Rect.left;
	iWindowSizeY = Rect.bottom - Rect.top;
	
	// Регистрируем класс окна
	if (!RegisterClass(&wc))
	{
		//MessageBox(NULL, "Не могу зарегистрировать класс окна!", "Ошибка", MB_OK);
		return 1;
	}
	
	printf("CreateWindow() ");
	// Создаем основное окно приложения
	hWnd = CreateWindow( 
			szClassName,                // Имя класса
			"LasCon Graphical Info",    // Текст заголовка 
			dwWindowStyle,              // Стиль окна
			50,   50,                   // Позиция левого верхнего угла
			iWindowSizeX,  iWindowSizeY,// Ширина и высота окна
			(HWND) NULL,                // Указатель на родительское окно NULL
			(HMENU) NULL,               // Используется меню класса окна
			(HINSTANCE)hInstance,       // Указатель на текущее приложение
			NULL);                      // Передается в качестве lParam в событие WM_CREATE
	if (!hWnd) 
	{
		MessageBox(NULL, "Не удается создать главное окно!", "Ошибка", MB_OK);
		return 2;
	}

	printf("OK!\r\n");

	// Показываем наше окно
	ShowWindow(hWnd, SW_SHOW); 
	UpdateWindow(hWnd);

	// Выполняем цикл обработки сообщений до закрытия приложения
	while (GetMessage(&lpMsg, NULL, 0, 0))
	{
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}
	
	free(frame_buffer);
	
	return 0;
}

int CopyFrameBuffer(HDC hdc, RECT *r)
{
	HBITMAP hbitmap;
	HDC hdct;
	BITMAPINFO bmi;
	VOID *pvbits;

	int h = r->bottom - r->top;
	int w = r->right - r->left;

	ZeroMemory(&bmi,sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biHeight = winy;
	bmi.bmiHeader.biWidth  = winx;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;

	hdct = CreateCompatibleDC(NULL);
	hbitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &pvbits, NULL,0x0);
	SelectObject(hdct, hbitmap);

	memcpy(pvbits, frame_buffer, h*w*4);

	BitBlt(hdc, r->left, r->top, r->right - r->left, r->bottom - r->top, hdct, 0, 0, SRCCOPY);

	DeleteObject(hbitmap);
	DeleteDC(hdct);

	return 1;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	RECT Rect;
	HDC hdc;
	
	switch (messg)
	{
	case WM_CREATE:
		DBG("WM_CREATE,%04x,%04x\r\n", wParam, lParam);
		GetClientRect(hWnd, &Rect);
		winx = Rect.right - Rect.left;
		winy = Rect.bottom - Rect.top;
		break;
		
	case WM_PAINT:
		DBG("WM_PAINT,%04x,%04x\r\n", wParam, lParam);
		hdc = GetDC(hWnd);
		GetClientRect(hWnd, &Rect);
		CopyFrameBuffer(hdc, &Rect);
		ValidateRect(hWnd, &Rect);
		break;

	case WM_COMMAND: 
		DBG("WM_COMMAND,%04x,%04x\r\n", wParam, lParam);
		switch (LOWORD(wParam)) 
		{
		default:
			break;
		} 
		break;

	case WM_KEYDOWN:
		DBG("WM_KEYDOWN,%04x,%04x\r\n", wParam, lParam);
		switch (wParam)
		{
		case VK_LEFT:
			break;
		case VK_RIGHT:
			break;
		case VK_UP:
			break;
		case VK_DOWN:
			break;
		case VK_HOME:
			break;
		case VK_END:
			break;
		case VK_INSERT:
			break;
		case VK_DELETE:
			break;
		case VK_F2:
			break;
		case VK_ESCAPE:
			goto win_exit;
		default:
			break;
		}
		break;
	
	case WM_DESTROY:
win_exit:
		fKillThread = 1;
		WaitForSingleObject(hThread1, 5000); // max 5 sec
		WaitForSingleObject(hThread2, 5000); // max 5 sec
		CloseHandle(hThread1);
		CloseHandle(hThread2);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, messg, wParam, lParam);
	}
	return 0;
}

void put_label(float grad, int color)
{
	grad = ((float)grad*PI)/180;

	_putline(SIZE_X/2+sinf(grad)*(SIZE_X/2-8), SIZE_X/2+10-cosf(grad)*(SIZE_X/2-8),
		SIZE_X/2+sinf(grad)*(SIZE_X/2-2), SIZE_X/2+10-cosf(grad)*(SIZE_X/2-2),
		color);
}

void put_compass(float grad, int color)
{
	float grad_a = ((grad+80)*PI)/180;
	float grad_b = ((grad-80)*PI)/180;
	grad = ((float)grad*PI)/180;

	_putline(SIZE_X/2+sinf(grad_a)*(15), SIZE_X/2+10-cosf(grad_a)*(15),
		SIZE_X/2+sinf(grad_b)*(15), SIZE_X/2+10-cosf(grad_b)*(15),
		color);
	_putline(SIZE_X/2+sinf(grad_a)*(15), SIZE_X/2+10-cosf(grad_a)*(15),
		SIZE_X/2+sinf(grad)*(SIZE_X/2-10), SIZE_X/2+10-cosf(grad)*(SIZE_X/2-10),
		color);
	_putline(SIZE_X/2+sinf(grad_b)*(15), SIZE_X/2+10-cosf(grad_b)*(15),
		SIZE_X/2+sinf(grad)*(SIZE_X/2-10), SIZE_X/2+10-cosf(grad)*(SIZE_X/2-10),
		color);
}

//extern int get_imu_data(pimu_data_t imu);
//extern int get_distantion(int *mm);
//int get_distantion_start(void);
//int get_distantion_end(int *mm);
//int laser_on(void);

#define NO_DIST
//#define ONLY_GRAPH_TEST 

DWORD WINAPI ProcGenerateFrame( LPVOID lpParam )
{
	int i;
	int ret;
	int k = 0;
	char str[128];
	
	imu_data_t imu;
	int mm;
	int dist_process = 0;
	
	int error = 0;
	
	float tilt = 0.0f;
	float azimuth = 0.0f; //180.0f;
	
	printf("ProcGenerateFrame()\r\n");

#if !defined( NO_DIST )
	// Получение дистанции, если таймаут, требуется подать команду на включение.
	ret = get_distantion(&mm);
	if (ret == 4)
	{
		laser_on();
		Sleep(1000);
	}
	else
	{
		Sleep(100);
	}
#endif
	
	while (1)
	{
		printf("(.)");
	
#if defined( ONLY_GRAPH_TEST )
		imu.tilt = 12.2f;
		imu.azimuth = 243.7f;
		imu.status = 0;
		mm = 27895;
#else
		// Получение данных с устройства:
		ret = get_imu_data(&imu);
		if (ret)
		{
			error = 1;
		}
#if !defined( NO_DIST )
//		// Дистанция обновляется только каждый 5ый раз:
//		if ((k % 5) == 0)
//		{
//			// Получаем расстояние:
//			ret = get_distantion(&mm);
//			if (ret)
//			{
//				error = 2;
//			}
//		}
		// Работаем не блокирующими функциями!
		if (dist_process == 0)
		{
			printf("get_distantion_start()\r\n");
			ret = get_distantion_start();
			if (ret)
			{
				error = 2;
			}
			else
			{
				dist_process = 20;
			}
		}
		else
		{
			printf("get_distantion_end()\r\n");
			ret = get_distantion_end(&mm);
			printf("ret = %d, mm = %d\r\n", ret, mm);
			if (ret == 1 || ret == 2)
			{
				//error = 2;
				dist_process--;
				if (dist_process == 0)
				{
					// Если dist_process доходит но нуля, считаем что случился таймаут!
					mm = -1;
				}
			}
			else if (ret == -1)
			{
				error = 2;
				dist_process = 0;
			}
			else if (ret == 0)
			{
				dist_process = 0;
			}
			else
			{
				mm = -1;
				dist_process = 0;
				//error = 2;
			}
		}
#else
		else
		{
			error = 0;
			mm = 0;
		}
#endif
		printf("imu.status = %d\r\n", imu.status);
#endif
		if (imu.status == 0 || 
			imu.status == 2)
		{
			if (ABS(azimuth - imu.azimuth) > 180.0f)
			{
				if (imu.azimuth < 90.0f)
					imu.azimuth += 360.0f;
				else if (azimuth < 90.0f)
					azimuth += 360.0f;
			}
			
			if (ABS(azimuth - imu.azimuth) < 5.0f) // if noise
				azimuth = lowpass(imu.azimuth, 0.2f, azimuth);
			else
				azimuth = imu.azimuth;
			
			if (ABS(tilt - imu.tilt) < 5.0f) // if noise
				tilt    = lowpass(imu.tilt, 0.2f, tilt);
			else
				tilt    = imu.tilt;
			
			if (azimuth >= 360.0f)
				azimuth -= 360.0f;
		}
		else
		{
			error = imu.status;
		}
		
		// Обнуляем кадровый буфер:
		for (i = 0; i < winx*winy; i++)
		{
			frame_buffer[i] = 0x00000000;
		}
		
		if (error == 0)
		{
			// Рисуем окружность компаса:
			_putcircle(SIZE_X/2,SIZE_X/2+10,SIZE_X/2-5,0,0x00FFFFFF);
			
			// Рисуем риски:
			for (i = 0; i < 360; i += 15)
			{
				put_label(i, 0x00FFFFFF);
			}
			
			if (imu.status != 2)
			{
				// Рисуем стрелку компаса:
				put_compass((int)azimuth+180, 0x00FF0000);
				put_compass((int)azimuth+  0, 0x0000AAFF);
			}
			
			/*
			if (imu.status == 1)
			{
				_putstr(60, SIZE_X/2, (unsigned char*)"Прибор в движении!", 0x00FFFFFF);
			}
			else if (tilt >  89.0f || tilt < -89.0f)
			{
				_putstr(60, SIZE_X/2, (unsigned char*)"Направление вверх!", 0x00FFFFFF);
			}
			*/
			
			// Выводим текстовую информацию:
			if ((tilt >  89.0f || tilt < -89.0f) ||
				(imu.status == 1))
			{
				_sprintf(str, "Азимут: -");
			}
			else
			{
				_sprintf(str, "Азимут: %3d.%d°", (int)azimuth, (int)(azimuth*10)%10);
			}
			_putstr(90, 260, (unsigned char*)str, 0x00FFFFFF);
			if (imu.status == 1)
			{
				_sprintf(str, "Угол к горизонту:  -");
			}
			else
			{
				_sprintf(str, "Угол к горизонту: %3d.%d°", (int)tilt, ABS((int)(tilt*10)%10));
			}
			_putstr(10, 280, (unsigned char*)str, 0x00FFFFFF);
			if (error == 2 ||
				mm == 0 ||
				mm == -1)
			{
				_sprintf(str, "Расстояние:  -");
				_putstr(58, 300, (unsigned char*)str, 0x00FFFFFF);
			}
			else
			{
				_sprintf(str, "Расстояние: %3d.%d%dм", (int)mm/1000, (int)(mm/100)%10, (int)(mm/10)%10);
				_putstr(58, 300, (unsigned char*)str, 0x00FFFFFF);
			}
			error = 0;
		}
		else if (error == 1)
		{
			_putstr(10, SIZE_Y/2-10, (unsigned char*)"Ошибка получения ориентации!", 0x00FFFFFF);
		}
		else if (error == 2)
		{
			_putstr(13, SIZE_Y/2-10, (unsigned char*)"Ошибка получения дистанции!", 0x00FFFFFF);
		}
		else if (error == 3)
		{
			_putstr(40, SIZE_Y/2-10, (unsigned char*)"Прибор в движении!", 0x00FFFFFF);
			error = 0;
		}
		else
		{
			_putstr(40, SIZE_Y/2-10, (unsigned char*)"Неизвестная ошибка!", 0x00FFFFFF);
		}
		
		// Команда на обновление окна:
		InvalidateRect(hWnd, 0, TRUE);
		UpdateWindow(hWnd);
		
		//if (error == 0)
		Sleep(200);
		//else
		//	break;
		
		if (error == 2)
		{
			error = 0;
		}
		
		if (fKillThread)
			break;
		
		k++;
	}
	return 0; 
}

DWORD WINAPI ProcGetData( LPVOID lpParam )
{
	printf("ProcGetData()\r\n");
	while (1)
	{
		printf(".");
		
		Sleep(1000);
		
		if (fKillThread)
			break;
	}
	return 0;
}
#endif // GRAPH_MODE
