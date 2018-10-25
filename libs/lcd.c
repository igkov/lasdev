#include <stdint.h>
#include <string.h>
#include "lcd.h"
#if !defined( WIN32 )
#include "spi9.h"
#endif

/* (32+9)*9 = 41*9 = 369 / 8 = 46.125, округляется до макс. */
static uint8_t lcd_frame[47] = 
{
	0xFC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
}; // todo - :21 - :28

static const uint16_t segs[] = 
{
	// line 1
	LCD_LINE1_SEG1_A, LCD_LINE1_SEG1_B, LCD_LINE1_SEG1_C, LCD_LINE1_SEG1_D, LCD_LINE1_SEG1_E, LCD_LINE1_SEG1_F, LCD_LINE1_SEG1_G, 
	LCD_LINE1_SEG2_A, LCD_LINE1_SEG2_B, LCD_LINE1_SEG2_C, LCD_LINE1_SEG2_D, LCD_LINE1_SEG2_E, LCD_LINE1_SEG2_F, LCD_LINE1_SEG2_G, 
	LCD_LINE1_SEG3_A, LCD_LINE1_SEG3_B, LCD_LINE1_SEG3_C, LCD_LINE1_SEG3_D, LCD_LINE1_SEG3_E, LCD_LINE1_SEG3_F, LCD_LINE1_SEG3_G, 
	LCD_LINE1_SEG4_A, LCD_LINE1_SEG4_B, LCD_LINE1_SEG4_C, LCD_LINE1_SEG4_D, LCD_LINE1_SEG4_E, LCD_LINE1_SEG4_F, LCD_LINE1_SEG4_G, 
	LCD_LINE1_SEG5_A, LCD_LINE1_SEG5_B, LCD_LINE1_SEG5_C, LCD_LINE1_SEG5_D, LCD_LINE1_SEG5_E, LCD_LINE1_SEG5_F, LCD_LINE1_SEG5_G,
	// line 2
	LCD_LINE2_SEG1_A, LCD_LINE2_SEG1_B, LCD_LINE2_SEG1_C, LCD_LINE2_SEG1_D, LCD_LINE2_SEG1_E, LCD_LINE2_SEG1_F, LCD_LINE2_SEG1_G, 
	LCD_LINE2_SEG2_A, LCD_LINE2_SEG2_B, LCD_LINE2_SEG2_C, LCD_LINE2_SEG2_D, LCD_LINE2_SEG2_E, LCD_LINE2_SEG2_F, LCD_LINE2_SEG2_G, 
	LCD_LINE2_SEG3_A, LCD_LINE2_SEG3_B, LCD_LINE2_SEG3_C, LCD_LINE2_SEG3_D, LCD_LINE2_SEG3_E, LCD_LINE2_SEG3_F, LCD_LINE2_SEG3_G, 
	LCD_LINE2_SEG4_A, LCD_LINE2_SEG4_B, LCD_LINE2_SEG4_C, LCD_LINE2_SEG4_D, LCD_LINE2_SEG4_E, LCD_LINE2_SEG4_F, LCD_LINE2_SEG4_G,  
	LCD_LINE2_SEG5_A, LCD_LINE2_SEG5_B, LCD_LINE2_SEG5_C, LCD_LINE2_SEG5_D, LCD_LINE2_SEG5_E, LCD_LINE2_SEG5_F, LCD_LINE2_SEG5_G, 
	// line 3
	LCD_LINE3_SEG1_A, LCD_LINE3_SEG1_B, LCD_LINE3_SEG1_C, LCD_LINE3_SEG1_D, LCD_LINE3_SEG1_E, LCD_LINE3_SEG1_F, LCD_LINE3_SEG1_G, 
	LCD_LINE3_SEG2_A, LCD_LINE3_SEG2_B, LCD_LINE3_SEG2_C, LCD_LINE3_SEG2_D, LCD_LINE3_SEG2_E, LCD_LINE3_SEG2_F, LCD_LINE3_SEG2_G, 
	LCD_LINE3_SEG3_A, LCD_LINE3_SEG3_B, LCD_LINE3_SEG3_C, LCD_LINE3_SEG3_D, LCD_LINE3_SEG3_E, LCD_LINE3_SEG3_F, LCD_LINE3_SEG3_G, 
	LCD_LINE3_SEG4_A, LCD_LINE3_SEG4_B, LCD_LINE3_SEG4_C, LCD_LINE3_SEG4_D, LCD_LINE3_SEG4_E, LCD_LINE3_SEG4_F, LCD_LINE3_SEG4_G, 
	LCD_LINE3_SEG5_A, LCD_LINE3_SEG5_B, LCD_LINE3_SEG5_C, LCD_LINE3_SEG5_D, LCD_LINE3_SEG5_E, LCD_LINE3_SEG5_F, LCD_LINE3_SEG5_G, 
}; // 7 * 15 * 2 = 210b

static const uint8_t syms[] = 
{
	      // SEG -GFEDCBA
	0x3F, // 0 - 00111111
	0x06, // 1 - 00000110
	0x5B, // 2 - 01011011
	0x4F, // 3 - 01001111
	0x66, // 4 - 01100110
	0x6D, // 5 - 01101101
	0x7D, // 6 - 01111101
	0x07, // 7 - 00000111
	0x7F, // 8 - 01111111
	0x6F, // 9 - 01101111
	0x77, // A - 01110111
	0x7C, // b - 01111100
	0x39, // C - 00111001
	0x5E, // d - 01011110
	0x79, // E - 01111001
	0x71, // F - 01110001
	0x50, // r - 01010000
	0x38, // L - 00111000
	0x78, // t - 01111000
	0x73, // P - 01110011
	0x00, // SPACE 00000000
	0x40, // MINUS 01000000 
};

// Заполнение символа батареи:
const int16_t lcd_arr_batt[] = { LCD_BATT_SEG3, LCD_BATT_SEG2, LCD_BATT_SEG1, LCD_BATT_COVER, -1 }; // 3, 2, 1, BATT
// Заполнение уровня приема:
const int16_t lcd_las_level[] = { -1 };
// Вывод пинктограммы лазера:
const int16_t lcd_arr_las[]  = { LCD_SETUP_BODY, LCD_SETUP_LAS, -1 };
// Вывод единиц измерения:
const int16_t lcd_unit[]  = { 
	LCD_LINE1_N, LCD_LINE1_N1, 
	LCD_LINE2_SEGX_A, LCD_LINE2_SEGX_B, LCD_LINE2_SEGX_F, LCD_LINE2_SEGX_G, 
	LCD_LINE3_SEGX_A, LCD_LINE3_SEGX_B, LCD_LINE3_SEGX_F, LCD_LINE3_SEGX_G, 
	LCD_LINE1_DP1, LCD_LINE2_DP3, LCD_LINE3_DP3, -1 };

//int16_t lcd_hello[] = { 15, 11, 13, 16, 12, 20, 26, 24, 9, 25, 35, 18, 34, 44, 27, 43, 53, 47, 49, 36, 52, 50, -1 };
//int16_t lcd_err[]   = { 20, 26, 24, 9, 25, 18, 33, 27, 42, -1 };
const int16_t lcd_cal[]   = { 
	LCD_LINE1_SEG2_A, LCD_LINE1_SEG2_F, LCD_LINE1_SEG2_E, LCD_LINE1_SEG2_D, 
	LCD_LINE1_SEG3_A, LCD_LINE1_SEG3_F, LCD_LINE1_SEG3_E, LCD_LINE1_SEG3_C, LCD_LINE1_SEG3_B, LCD_LINE1_SEG3_G, 
	LCD_LINE1_SEG4_F, LCD_LINE1_SEG4_E, LCD_LINE1_SEG4_D, -1 };
const int16_t lcd_of_32[] = { 
	LCD_LINE2_SEG1_G, LCD_LINE2_SEG1_C, LCD_LINE2_SEG1_E, LCD_LINE2_SEG1_D, 
	LCD_LINE2_SEG2_A, LCD_LINE2_SEG2_F, LCD_LINE2_SEG2_E, LCD_LINE2_SEG2_G, 
	LCD_LINE2_SEG4_A, LCD_LINE2_SEG4_B, LCD_LINE2_SEG4_C, LCD_LINE2_SEG4_D, LCD_LINE2_SEG4_G, 
	LCD_LINE2_SEG5_A, LCD_LINE2_SEG5_B, LCD_LINE2_SEG5_G, LCD_LINE2_SEG5_E, LCD_LINE2_SEG5_D, -1 };
const int16_t lcd_end[]   = { 
	LCD_LINE2_SEG2_A, LCD_LINE2_SEG2_F, LCD_LINE2_SEG2_E, LCD_LINE2_SEG2_G, LCD_LINE2_SEG2_D, 
	LCD_LINE2_SEG3_E, LCD_LINE3_SEG2_G, LCD_LINE3_SEG2_C, 
	LCD_LINE2_SEG4_E, LCD_LINE2_SEG4_G, LCD_LINE2_SEG4_C, LCD_LINE2_SEG4_B, LCD_LINE2_SEG4_D, 
	-1 };

// Вывод символа "ключ":
const int16_t lcd_setup[] = { LCD_SETUP_MAIN, -1 };

// Заполнение строки минусами:
const int16_t lcd_line_1_nop[] = { 
	LCD_LINE1_SEG1_G, LCD_LINE1_SEG2_G, LCD_LINE1_SEG3_G, LCD_LINE1_SEG4_G, LCD_LINE1_SEG5_G, -1 };
const int16_t lcd_line_2_nop[] = { 
	LCD_LINE2_SEG1_G, LCD_LINE2_SEG2_G, LCD_LINE2_SEG3_G, LCD_LINE2_SEG4_G, LCD_LINE2_SEG5_G, -1 };
const int16_t lcd_line_3_nop[] = { 
	LCD_LINE3_SEG1_G, LCD_LINE3_SEG2_G, LCD_LINE3_SEG3_G, LCD_LINE3_SEG4_G, LCD_LINE3_SEG5_G, -1 };

// Вывод сообщения Error:
const int16_t lcd_line_1_err[]   = { 
	LCD_LINE1_SEG2_A, LCD_LINE1_SEG2_D, LCD_LINE1_SEG2_F, LCD_LINE1_SEG2_E, LCD_LINE1_SEG2_G, 
	LCD_LINE1_SEG3_G, LCD_LINE1_SEG3_E, 
	LCD_LINE1_SEG4_G, LCD_LINE1_SEG4_E, -1 };
const int16_t lcd_line_2_err[]   = { 
	LCD_LINE2_SEG2_A, LCD_LINE2_SEG2_D, LCD_LINE2_SEG2_F, LCD_LINE2_SEG2_E, LCD_LINE2_SEG2_G, 
	LCD_LINE2_SEG3_G, LCD_LINE2_SEG3_E, 
	LCD_LINE2_SEG4_G, LCD_LINE2_SEG4_E, -1 };
const int16_t lcd_line_3_err[]   = { 
	LCD_LINE3_SEG2_A, LCD_LINE3_SEG2_D, LCD_LINE3_SEG2_F, LCD_LINE3_SEG2_E, LCD_LINE3_SEG2_G, 
	LCD_LINE3_SEG3_G, LCD_LINE3_SEG3_E, 
	LCD_LINE3_SEG4_G, LCD_LINE3_SEG4_E, -1 };

static void put_dec(int line, int seg, int value)
{
	int i;
	int addr;
	for (i=0; i < 7; i++)
	{
		addr = segs[(line*5+seg)*7+i];
		if (syms[value] & (1<<i))
			LCD_SET_BIT(addr)
		else
			LCD_CLR_BIT(addr)
	}
}

static void put_munis(int line, int flag)
{
	switch (line)
	{
	case 0:
		if (flag)
			LCD_SET_BIT(LCD_LINE1_MINUS)
		else
			LCD_CLR_BIT(LCD_LINE1_MINUS)
		break;
	case 1:
		if (flag)
			LCD_SET_BIT(LCD_LINE2_MINUS)
		else
			LCD_CLR_BIT(LCD_LINE2_MINUS)
		break;
	case 2:
		if (flag)
			LCD_SET_BIT(LCD_LINE3_MINUS)
		else
			LCD_CLR_BIT(LCD_LINE3_MINUS)
		break;
	}
}

void put_clr(int line)
{
	put_dec(line, 0, LCD_SYMBOL_SPACE);
	put_dec(line, 1, LCD_SYMBOL_SPACE);
	put_dec(line, 2, LCD_SYMBOL_SPACE);
	put_dec(line, 3, LCD_SYMBOL_SPACE);
	put_dec(line, 4, LCD_SYMBOL_SPACE);
	put_munis(line, 0);
}

static void put_int(int line, int value, int point)
{
	int ch;
	int sign = 0;
	// clr
	put_clr(line);
	// sign
	if (value < 0)
	{
		sign = 1;
		value = -value;
	}
	ch = value % 10;
	value /= 10;
	put_dec(line, 4, ch);
	if (value == 0 &&
		point < 1)
	{
		if (sign)
			put_dec(line, 3, LCD_SYMBOL_MINUS);
		return;
	}
	ch = value % 10;
	value /= 10;
	put_dec(line, 3, ch);
	if (value == 0 &&
		point < 2)
	{
		if (sign)
			put_dec(line, 2, LCD_SYMBOL_MINUS);
		return;
	}
	ch = value % 10;
	value /= 10;
	put_dec(line, 2, ch);
	if (value == 0 &&
		point < 3)
	{
		if (sign)
			put_dec(line, 1, LCD_SYMBOL_MINUS);
		return;
	}
	ch = value % 10;
	value /= 10;
	put_dec(line, 1, ch);
	if (value == 0 &&
		point < 4)
	{
		if (sign)
			put_dec(line, 0, LCD_SYMBOL_MINUS);
		return;
	}
	ch = value % 10;
	value /= 10;
	put_dec(line, 0, ch);
	if (sign)
		put_munis(line, 1);
	return;
}

void put_tilt(int value)
{
	put_int(1, value, 1);
}

void put_azimuth(int value)
{
	put_int(2, value, 1);
}

void put_dist(int value)
{
	put_int(0, value, 3);
}

void put_cnt(int value)
{
	put_int(2, value, 0);
}

uint16_t get_lcd_word(int n)
{
	uint16_t ret;
	int pos = (n * 9) / 8;
	int offset = (n * 9) % 8;
	ret = ((uint16_t)lcd_frame[pos] >> offset) | ((uint16_t)lcd_frame[pos+1] << (8-offset));
	return ret & 0x1FF; // 9 bit data
}

void set_frame(uint8_t *array, int size)
{
	if (size < sizeof(lcd_frame))
	{
		memcpy(lcd_frame, array, size);
	}
	else
	{
		memcpy(lcd_frame, array, sizeof(lcd_frame));
	}
}

#if defined( WIN32 )
void lcd_bit_access(int pos, int flag)
{
	if (flag)
		LCD_SET_BIT(pos)
	else
		LCD_CLR_BIT(pos)
}

void get_frame(uint8_t *array, int size)
{
	if (size < sizeof(lcd_frame))
	{
		memcpy(array, lcd_frame, size);
	}
	else
	{
		memcpy(array, lcd_frame, sizeof(lcd_frame));
	}
}
#else
void send_null_frame(void)
{
	int n;
	spi9_select();
	//for (n = 0; n < (32+9); n++)
	for (n = 0; n < (1); n++)
	{
		spi9_send(0x0000);
	}
	spi9_unselect();
}

void send_frame(void)
{
	int n;
	spi9_select();
	for (n = 0; n < (32+9); n++)
	{
		spi9_send(get_lcd_word(n));
	}
	spi9_unselect();
}
#endif

void lcd_arr(const int16_t *arr, int flag)
{
	int i = 0;
	while (1)
	{
		if (arr[i] == -1)
			break;
		if (arr[i] < sizeof(lcd_frame)*8)
		{
			if (flag)
				LCD_SET_BIT(arr[i])
			else
				LCD_CLR_BIT(arr[i])
		}
		i++;
	}
}

