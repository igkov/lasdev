#ifndef __DRAW_H__
#define __DRAW_H__

#define SIZE_X 240
#define SIZE_Y 320

void _putpixel(int x, int y, int value);
int _putchar(int x, int y, unsigned char sym, int value);
int _putstr(int x, int y, unsigned char *str, int value);
void _putline(int x1, int y1, int x2, int y2, int color);
void _putcircle(int x, int y, int radius, char fill, int color);

#endif //__DRAW_H__
