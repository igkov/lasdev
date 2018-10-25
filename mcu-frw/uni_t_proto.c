#include <stdint.h>

#include "uart.h"
#include "uni_t_proto.h"
#include "event.h"
#include "gio_lpc13xx.h"

void uni_t_init(void)
{
	// Инициализация пинов:
	// a. Пин выключения.
	//gio_out(3,0);
	//gio_set1(3,0);
	// b. Пин включения/измерения.
	gio_in(3,3);
	//gio_set0(3,3);
}

// Command:
// *00001#
// Answer:
// pMsgWrite TRUE 
// pInitDataWrite TRUE

int command_00001(void)
{
	return uart_command("*00001#\r\n", 9);
}

// Command:
// *00002#
// Takes 3 readings, screen shows "Er". Outputs the 3 measurements in array notation (distance is 103.7mm):
// Answer:
// u32Dist[0]=1037  u32Dist[1] =1037 u32Dist[2] =1037
// u32temp =0
// *000720150000000042#

int command_00002(void)
{
	return uart_command("*00002#\r\n", 9);
}

// Command:
// *100515#
// Turns on laser light (seems to freeze the screen, and the buttons no longer work).
// Answer:
// ???

int laser_on(void)
{
	return uart_command("*100515#\r\n", 10);
}

// Command:
// *00004#
// Takes 1 measurement, screen shows the measurement. Outputs the measurement like this: (distance is 112.7mm)
// Answer:
// Dist: 1127,curtemp =22 
// V2.0 
// nDist: 1127,tempDv=0
// *0006400000112784#

int get_dist_start(void)
{
	uart_command("*00004#\r\n", 9);
	// Здесь задержка около 1 секунды.
	// В данный момент логично проводить измерения и вычисления IMU-ядра!
	return 0;
}

int start_receive(void)
{
	// Отправляем "пустую команду", фактически ничего не отправляется.
	// Просто переводится машина состояний протокола в активный прием.
	uart_command((void*)0, 0);
	return 0;
}

int get_dist_check(void)
{
	return uart_answer_check();
}

#if 0
int get_dist_end(int *dist)
{
	int len = 16;
	int ret;
	uint8_t data[16];
	//uart_command("*00004#\r\n", 9);
	//answer *0006400000112784#
	//       *00084500750000060539#
	//       *00084500390000060199#
	ret = uart_answer(data, &len, 5000);
	if (ret)
		return ret;
	if (data[0] == 0 && 
		data[1] == 6 && 
		data[2] == 40)
	{
		*dist = (data[3]*1000000 + data[4]*10000 + data[5]*100 + data[6]);
		return 0;
	}
	else if(data[0] == 0 && 
			data[1] == 8 && 
			data[2] == 45)
	{
		*dist = (data[5]*1000000 + data[6]*10000 + data[7]*100 + data[8]);
		return 0;
	}
	else
	{
		return 2;
	}
}
#endif

/*
	Bootup information:

	ldpara:395
	curent ver:420411
	Year:Jan 21 2013 Time:13:53:10
	ldpara:395
	Iint OK
	APDMIN=136 APDMAX=167
	BIASVOLMIN=2307 BIASVOLMAX =1718

	To turn the unit on, connect the ON signal to GND for about 300ms. 
	Once it's on, the ON signal is also used to take a measurement. 
	After a measurement is taken, the unit outputs the following:
	
	Dist: 2827,curtemp =21
	V2.0
	nDist: 2827,tempDv=0

	The Dist and nDist values are in millimeters. As far as I can tell, 
	the two measurements are always identical. If there's a measurement 
	error (Error 154, out of range or sensor error) the unit will output:

	OUT_RAN dist = 30

	If the unit can't determine the average distance (Error 160, sensor 
	shaking too much or Error 155, signal too weak) the unit will output 
	one of the following lines:

	MEDIUM2 AND THIN NOT MATCH
	MEDIUM1 AND MEDIUM2 NOT MATCH
	THICK AND MEDIUM1 NOT MATCH
	MEDIUM1 AND MEDIUM2 NOT MATCH

	When you turn it off, it outputs:

	SysPowerOff!
	WriteTestData TRUE
 */
void power_on(void)
{
	gio_out(3,3);
	gio_set1(3,3);
	delay_ms(800);
	gio_set0(3,3);
	gio_in(3,3);
}

/*
	 sending 0x23 ('#') or 0x73 ('r') will cause the unit to power down.
 */
void power_down(void)
{
	// Работает только если не было команд.
	// Иначе повторяет последнюю команду.
	uart_putchar(0x23);
	uart_putchar(0x73);
	delay_ms(100);
	//while (1);
}

