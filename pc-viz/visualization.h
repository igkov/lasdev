#ifndef __VISUALIZATION_H__
#define __VISUALIZATION_H__

#define _WIN32_WINNT 0x500
#include <windows.h>
//#include <windowsx.h>


#define VIS_COLOR_WHITE   0x00
#define VIS_COLOR_BLUE    0x01
#define VIS_COLOR_GREEN   0x02
#define VIS_COLOR_RED     0x03
#define VIS_COLOR_AQUA    0x04
#define VIS_COLOR_PURP    0x05
#define VIS_COLOR_YELLOW  0x06
#define VIS_COLOR_MAX     0x07

typedef struct
{
	int flag;
	int stat;
	
	int sizex;
	int sizey;
	
	HWND hWnd;
	HINSTANCE hInstance;
	HDC hCompatibleDC;
	
	HANDLE hThread;
	
	HANDLE hBitmap, hOldBitmap;
	PAINTSTRUCT PaintStruct;

	float *pfData[VIS_COLOR_MAX];
	//float *pfData;
	int offset;
	int len;
	float zero;
	float scale;
	
	char *pszName;
	
} visual_t, *pvisual_t;

int graph_init(void);

// Создание окна визуализации с графиком:
int graph_create(pvisual_t vis, int sizex, int sizey, char *name, float scale, float zero);

int graph_scale(pvisual_t vis, float scale, float zero);

// Добавление точки на график:
int graph_add(pvisual_t vis, float value, int color);

// Добавление текста:
int graph_text(pvisual_t vis, char *name);

// Переход на следующую позицию графика:
int graph_next(pvisual_t vis, float value, int color);

// Очистка графика:
int graph_clear(pvisual_t vis);

// Уничтожение объекта графика:
int graph_destroy(pvisual_t vis);

#endif // __VISUALIZATION_H__
