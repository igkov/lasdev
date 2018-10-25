/*
	Модуль визуалиции.
	
	TODO:
	* Стиль рисования: точки, линии.
	* Отдельная функция сдвига: разделить add и inc.
	* Уничтожение окон графиков: очистка памяти.
	* Вывод текстовой строка с информацией в окне.
	* Тестовый вывод PID-контроллера.
	* Демонстрация фильтрации.
	* Удалить лишний код, причесать исходник.
	
	igorkov / 2014-2015 / fsp@igorkov.org
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "io.h"
#include "visualization.h"
#include "calibrate.h"

typedef struct
{
	HWND hWnd;
	pvisual_t vis;
} mapping_t, *pmapping_t;

DWORD dwColorTable[VIS_COLOR_MAX] = 
{
	RGB(0xFF,0xFF,0xFF),
	RGB(0x00,0x00,0xFF),
	RGB(0x00,0xFF,0x00),
	RGB(0xFF,0x00,0x00),
	RGB(0x00,0xFF,0xFF),
	RGB(0xFF,0x00,0xFF),
	RGB(0xFF,0xFF,0x00),
};

#define MAP_SIZE 32
mapping_t map[MAP_SIZE];

static int map_add(HWND hWnd, pvisual_t vis)
{
	int i = 0;
	while (i < MAP_SIZE)
	{
		if (map[i].hWnd == NULL)
		{
			map[i].hWnd = hWnd;
			map[i].vis = vis;
			return 0;
		}
		i++;
	}
	return 1;
}

static pvisual_t map_get(HWND hWnd)
{
	int i = 0;
	while (i < MAP_SIZE)
	{
		if (map[i].hWnd == hWnd)
		{
			return map[i].vis;
		}
		i++;
	}
	return NULL;
}

static int map_del(HWND hWnd)
{
	int i = 0;
	while (i < MAP_SIZE)
	{
		if (map[i].hWnd == hWnd)
		{
			map[i].hWnd = NULL;
			map[i].vis = NULL;
			return 0;
		}
		i++;
	}
	return 1;
}

static void create_image(pvisual_t vis, int x, int y)
{
	int i;
	HPEN hPen;
	RECT Rect;
	HDC hDC;
	int color;
	
	hDC = vis->hCompatibleDC;
	
	// Putting Dots (data):
	hPen = CreatePen(PS_DOT,1,RGB(0x00,0x00,0x00));
	SelectObject(hDC, hPen);
	FillRect(hDC, &Rect, (HBRUSH)hPen);
	DeleteObject(hPen);

	for (color=0;color<VIS_COLOR_MAX;color++)
	{
		for (i = 0; i < vis->len; i++)
		{
			float data;
			data = vis->pfData[color][i];
			if (isnormal(data))
				SetPixel(hDC, i, vis->sizey/2 - (int)((float)vis->sizey*(data-vis->zero)/(vis->scale*2)), dwColorTable[color]);
		}
	}
	
	// Putting Mag (axis):
	hPen = CreatePen(PS_DOT,1,RGB(0xFF,0xFF,0xFF));
	SelectObject(hDC, hPen);
	SetBkColor(hDC, RGB(0x00,0x00,0x00));

	MoveToEx(hDC, 0, y/2, NULL);
	LineTo(hDC, x-1, y/2);
	//MoveToEx(hDC, (y/8), (3*y)/5-(y/8), NULL);
	//LineTo(hDC, (y/8), (3*y)/5+(y/8));

	DeleteObject(hPen);
	GetStockObject(DC_PEN);

#if 0
	hPen = CreatePen(PS_DOT,1,RGB(0x00,0xFF,0x00));
	SelectObject(hDC, hPen);

	// Curr Vector:
	for (i = LOG_DEPTH-1; i > 0; i--)
	{
		if (log_data[i].flag)
		{
			int x_off, y_off;
			x_off = (log_data[i].mag_x * y) / (1<<11);
			y_off = (log_data[i].mag_y * y) / (1<<11);
			MoveToEx(hDC, (y/8),  (3*y)/5, NULL);
			LineTo(hDC, (y/8)+x_off,  (3*y)/5+y_off);
			break;
		}
	}

	DeleteObject(hPen);
	GetStockObject(DC_PEN);
#endif

	if (0)
	{
		char text[] = "Hello!";
		SetBkMode(hDC, TRANSPARENT);
		SetTextColor(hDC, RGB(0xFF,0xFF,0xFF));
		TextOut(hDC, 20, 40, text, strlen(text));     
	}
	
	return;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, UINT wParam, LONG lParam)
{
	pvisual_t vis;
	HDC hDC;
	RECT Rect;
	
	vis = map_get(hWnd);
	
	// если VIS = NULL, тогда ограниченный режим: не отрисовываем данные!
	// требуется подождать пока проинициализируется!
	
	switch ( Message )
	{
	case WM_PAINT:
#if 1
		hDC = GetDC(hWnd);
		GetClientRect(hWnd, &Rect);
		vis->hCompatibleDC = CreateCompatibleDC(hDC);
		vis->hBitmap = CreateCompatibleBitmap(hDC, Rect.right, Rect.bottom); 
		vis->hOldBitmap = SelectObject(vis->hCompatibleDC, vis->hBitmap);
		
		create_image(vis, Rect.right - Rect.left, Rect.bottom - Rect.top);
		
		//hDC = BeginPaint(hWnd, &vis->PaintStruct);
		BitBlt(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom, vis->hCompatibleDC, 0, 0, SRCCOPY);
		//EndPaint(hWnd, &vis->PaintStruct);
		
		DeleteObject(vis->hBitmap);
		DeleteDC(vis->hCompatibleDC);
		ReleaseDC(hWnd, hDC);
		
		vis->flag = 1;
#else
		hDC = GetDC(hWnd);
		GetClientRect(hWnd, &Rect);
		CopyFrameBuffer(hDC, &Rect);
#endif
		ValidateRect(hWnd, &Rect);
		return 0;
#if 0
	case WM_SIZE:
		SelectObject(vis->hCompatibleDC, vis->hOldBitmap);
		DeleteObject(vis->hBitmap);
		DeleteDC(vis->hCompatibleDC);
		
		hDC = GetDC(hWnd);
		GetClientRect(hWnd, &Rect);
		vis->hCompatibleDC = CreateCompatibleDC(hDC);
		vis->hBitmap = CreateCompatibleBitmap(hDC, Rect.right, Rect.bottom); 
		vis->hOldBitmap = SelectObject(vis->hCompatibleDC, vis->hBitmap);
		
		// TODO: resize
		//{
		//	float *newdata;
		//	float *olddata;
		//	int len;
		//	
		//	newdata = (float*)malloc(sizeof(float) * Rect.right);
		//	len = vis->sizex > Rect.right ? len vis->sizex: 
		//	memcpy(newdata, vis->pfData, sizeof(float) * vis->sizex)
		//	if (vis->len
		//	
		//	vis->sizex = Rect.right;
		//	vis->sizey = Rect.bottom;
		//}
		return 0;
#endif
	case WM_ERASEBKGND:
		return 1;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}

static DWORD WINAPI ThreadProc(PVOID lpParam)
{
	WNDCLASS WndClass;
	MSG Msg;
	int c;
	
	pvisual_t vis = (pvisual_t)lpParam;
	char szClassName[64];

	DWORD dwWindowStyle = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME);
	int iWindowSizeX, iWindowSizeY;
	RECT Rect;

	// Class Name Construct:
	sprintf(szClassName, "Logging View %08x", (uint32_t)&vis);
	
	// Instance:
	vis->hInstance = GetModuleHandle(NULL);
	// DC:
	vis->hCompatibleDC = NULL;
	// Logging Data:
	for (c=0;c<VIS_COLOR_MAX;c++)
	{
		vis->pfData[c] = (float*)malloc(sizeof(float) * vis->sizex);
	}
	vis->offset = 0;
	vis->len = 0;

	// Window Initialization:
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = vis->hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = szClassName;
	
	// Class Refister:
	if ( !RegisterClass(&WndClass) )
	{
		printf("Error 1!\n");
		// todo: set stat
		return 0;
	}
	
	Rect.left = 100;
	Rect.right = Rect.left + vis->sizex;
	Rect.top = 100;
	Rect.bottom = Rect.top + vis->sizey;
	AdjustWindowRect(&Rect, dwWindowStyle, FALSE);
	iWindowSizeX = Rect.right - Rect.left;
	iWindowSizeY = Rect.bottom - Rect.top;

	// Windows Creating:
	vis->hWnd = CreateWindow(szClassName, 
		vis->pszName, 
		dwWindowStyle, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		iWindowSizeX, 
		iWindowSizeY, 
		NULL, 
		NULL, 
		vis->hInstance, 
		NULL);
	if ( ! vis->hWnd )
	{
		printf("Error 2!\n");
		// todo: set stat
		return 0;
	}
	
	if (map_add(vis->hWnd, vis))
	{
		printf("Error 3!\n");
		// todo: deinit, set stat
		return 0;
	}
	
	ShowWindow(vis->hWnd, SW_SHOW);
	UpdateWindow(vis->hWnd);
	
	while ( GetMessage(&Msg, NULL, 0, 0) )
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	
	// todo: set flag
	
	return 0;
}

// Создание окна визуализации с графиком:
int graph_init(void)
{
	memset(map, 0x00, sizeof(map));
	return 0;
}

// Создание окна визуализации с графиком:
int graph_create(pvisual_t vis, int sizex, int sizey, char *name, float scale, float zero)
{
	int i = 0;
	
	vis->stat = 0;
	vis->flag = 0;
	vis->pszName = name;
	vis->sizex = sizex;
	vis->sizey = sizey;
	
	vis->scale = scale;
	vis->zero = zero;
	
	vis->hThread = CreateThread(NULL, 0, ThreadProc, (PVOID)vis, 0, NULL);
	if (vis->hThread == INVALID_HANDLE_VALUE)
	{
		return 2;
	}

	while (1)
	{
		Sleep(200);
		if (vis->flag)
		{
			InvalidateRect(vis->hWnd, NULL, TRUE);
			break;
		}
		if (i>20)
		{
			CloseHandle(vis->hThread);
			return 3;
		}
		i++;
	}
	
	return vis->stat;
}

int graph_scale(pvisual_t vis, float scale, float zero)
{
	vis->scale = scale;
	vis->zero = zero;
	return 0;
}

// Добавление точки на график:
int graph_add(pvisual_t vis, float value, int color)
{
	if (vis->len < vis->sizex)
	{
		vis->pfData[color][vis->offset] = value;
		
		if (color == 0)
		{
			vis->len++;
			vis->offset++;
		}
	}
	else if (vis->len == vis->sizex)
	{
		int i, c;
		if (color == 0)
		{
			for (c=0;c<VIS_COLOR_MAX;c++)
			{
				for (i=1;i<vis->sizex;i++)
				{
					vis->pfData[c][i-1] = vis->pfData[c][i];
				}
				vis->pfData[c][vis->sizex-1] = NAN;
			}
		}
		vis->pfData[color][vis->sizex-1] = value;
	}
	
	if (color == 0)
	{
		InvalidateRect(vis->hWnd, 0, TRUE);
		UpdateWindow(vis->hWnd);
	}

	return 0;
}

// Добавление текста:
int graph_text(pvisual_t vis, char *name);

// Переход на следующую позицию графика:
int graph_next(pvisual_t vis, float value, int color);

// Очистка графика:
int graph_clear(pvisual_t vis);

// Уничтожение объекта графика:
int graph_destroy(pvisual_t vis);

#if defined( __TEST__ )

#if defined( __PID__ )
#include "pid.h"
#endif

int main(int argc, char **argv)
{
	int ret;
	int i = 0;
	
	visual_t vis1;
	visual_t vis2;

#if defined( __PID__ )	
	pid_struct_t pid1;
	pid_struct_t pid2;
#endif
	
	float targetPosition;
	float currentPosition;
	float speed = 0;
	
	graph_init();
	
	ret = graph_create(&vis1, 300, 100, "Test Graph 1", 1.0f, 0.0f);
	if (ret)
	{
		printf("graph_create (1) return %d!\n", ret);
	}
	ret = graph_create(&vis2, 640, 480, "Test Graph 2", 2.0f, 0.0f);
	if (ret)
	{
		printf("graph_create (2) return %d!\n", ret);
	}
	
#if defined( __PID__ )	
	pid_init(&pid1, 0.00005f, 4.0f, 300.0f, 10.0f);
#endif

	targetPosition = 1.0f;
	currentPosition = -1.0f;
	
	while (1)
	{
		float value;

#if defined( __PID__ )	
		#define dT 0.01f
		
		value = pid_update(&pid1, targetPosition, currentPosition);
		
		speed += value * dT;
		if (speed > 10.0f) speed = 10.0f;
		if (speed < -10.0f) speed = -10.0f;
		currentPosition += speed * dT;
		
		graph_add(&vis2, currentPosition, VIS_COLOR_RED);
		graph_add(&vis2, value/10, VIS_COLOR_YELLOW);
		graph_add(&vis2, speed, VIS_COLOR_GREEN);
		graph_add(&vis2, targetPosition, 0);
#else
		value = cosf((float)i/30);
		graph_add(&vis1, value, (i/10)%(VIS_COLOR_MAX-1)+1);
		value = sinf((float)i/20);
		graph_add(&vis1, value, 0);
		
		value = sinf((float)i/50);
		graph_add(&vis2, value, VIS_COLOR_GREEN);
		value = cosf((float)i/50);
		graph_add(&vis2, value, VIS_COLOR_RED);
		value = tanf((float)i/100);
		graph_add(&vis2, value, VIS_COLOR_YELLOW);
		graph_add(&vis2, NAN, VIS_COLOR_WHITE);
#endif

		if (i>600) Sleep(10);
		if (i>600) {
			if ((i%500)==0) {
				if (targetPosition == 1.0f) {
					targetPosition = 0.0f;
				} else {
					targetPosition = 1.0f;
				}
			}
		}
		i++;
	}
	
	Sleep(10000);
	
	return 0;
}
#endif
