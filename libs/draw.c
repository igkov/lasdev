#include <stdlib.h>
#include <stdint.h>
#include "printf.h"
#include "draw.h"

extern uint32_t *frame_buffer;
extern unsigned char symbol_table[256][12];

void _putpixel(int x, int y, int value)
{
	int j;
	j = x + (SIZE_Y-y-1)*SIZE_X;
	frame_buffer[j] = value;
}

int _putchar(int x, int y, unsigned char sym, int value)
{
	int i;
	int j;

	for (i = 0; i < 12; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if ((symbol_table[sym][i] & (0x01 << j)) == 0)
			{
				_putpixel(x+7-j, y+i, value);
			}
		}
	}

	return 0;
}

int _putstr(int x, int y, unsigned char *str, int value)
{
	while (*str != 0)
	{
		_putchar(x, y, *str, value);
		x += 8;
		if ((x+8) > SIZE_X)
		{
			x = 0;
			y += 12;
		}
		str++;
	}

	return 0;
}

#define ABS(a)   ((a)>0?(a):-(a))

void _putline(int x1, int y1, int x2, int y2, int color)
{
	int dy, dx;
	int addx = 1;
	int addy = 1;
	int P, diff;

	int i = 0;

	dx = ABS(x2 - x1);
	dy = ABS(y2 - y1);

	if(x1 > x2)
		addx = -1;
	if(y1 > y2)
		addy = -1;

	if(dx >= dy)
	{
		dy *= 2;
		P = dy - dx;
		diff = P - dx;

		for(; i <= dx; ++i)
		{
			_putpixel(x1, y1, color);
			if(P < 0)
			{
				P  += dy;
				x1 += addx;
			}
			else
			{
				P  += diff;
				x1 += addx;
				y1 += addy;
			}
		}
	}
	else
	{
		dx *= 2;
		P = dx - dy;
		diff = P - dy;

		for(; i <= dy; ++i)
		{
			_putpixel(x1, y1, color);
			if(P < 0)
			{
				P  += dx;
				y1 += addy;
			}
			else
			{
				P  += diff;
				x1 += addx;
				y1 += addy;
			}
		}
	}
}

void _putcircle(int x, int y, int radius, char fill, int color)
{
	int a, b, P;
	a = 0;
	b = radius;
	P = 1 - radius;
	do
	{
		if(fill)
		{
			_putline(x-a, y+b, x+a, y+b, color);
			_putline(x-a, y-b, x+a, y-b, color);
			_putline(x-b, y+a, x+b, y+a, color);
			_putline(x-b, y-a, x+b, y-a, color);
		}
		else
		{
			_putpixel(a+x, b+y, color);
			_putpixel(b+x, a+y, color);
			_putpixel(x-a, b+y, color);
			_putpixel(x-b, a+y, color);
			_putpixel(b+x, y-a, color);
			_putpixel(a+x, y-b, color);
			_putpixel(x-a, y-b, color);
			_putpixel(x-b, y-a, color);
		}
		if (P < 0)
		{
			P += 3 + 2 * a++;
		}
		else
		{
			P += 5 + 2 * (a++ - b--);
		}
	}
	while (a <= b);
}
