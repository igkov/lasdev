#ifndef __CALIBRATE_H__
#define __CALIBRATE_H__

#include "vector.h"

#define CAL_MIN_K 22
#define CAL_MAX_K 32
#define CAL_MEAN_K 6
#define CAL_MAX_REPEATE 64

#define MIN_ANGLE (20.0f / 180.0f * PI)

typedef struct
{
	int k;  // Указатель текущей заполняемой позизии.
	int kc; // Счетчик сохраненных позиций.
	int m;  // Счетчик значений на усреднении.
	vector3df_t values_a[CAL_MAX_K];
	vector3df_t values_m[CAL_MAX_K];
	calibrate_t cal_a;
	calibrate_t cal_m;
} calibrate_cntx_t, *pcalibrate_cntx_t;

/*
	calibration_next()

	Функция получения калибровочного значения.
	Внутри функции производится получение данных из датчика, 
	ожидание фиксации позиции, несложная фильтрация значений.
	
	Param:
	mag - указатель на структуру для сохранения данных с магнетометра.
	acc - указатель на структуру для сохранения данных с акселерометра.
	
	Return:
	0 - Нет ошибок.
	1 - Ошибка получения данных из датчиков.
	2 - Ошибка получения калибровочных данных, устройство в движении!
 */
int calibration_next(vector3df_t *acc, vector3df_t *mag);

/*
	calibration_calc()

	Вычисление калибровки по накопленным значениям в позициям.
	Производится решение переопределенной системы уравнений.
	
	Param:
	values_a   - накопленный массив значений в ориентациях
	values_m   - накопленный массив значений в ориентациях
	k          - количество значений в массиве
	pcal_a     - структура с калибровкой акселерометра
	pcal_m     - структура с калибровкой магнетометра
	
	Return:
	0 - Нет ошибок.
	1 - Ошибка, слишком много накоплено значений.
 */
int calibration_calc(pvector3df_t values_a, pvector3df_t values_m, int k, 
					 pcalibrate_t pcal_a, pcalibrate_t pcal_m);


int calibration_calc_int(pvector3d_t values_a, pvector3d_t values_m, int k, 
					 pcalibrate_t pcal_a, pcalibrate_t pcal_m);

/*
	calibration_calc9()

	Вычисление калибровки по накопленным значениям в позициям.
	Производится решение переопределенной системы уравнений.
	
	Param:
	values_a   - накопленный массив значений в ориентациях
	values_m   - накопленный массив значений в ориентациях
	k          - количество значений в массиве
	pcal_a     - структура с калибровкой акселерометра
	pcal_m     - структура с калибровкой магнетометра
	
	Return:
	0 - Нет ошибок.
	1 - Ошибка, слишком много накоплено значений.
 */
int calibration_calc9(pvector3df_t values_a, pvector3df_t values_m, int k, 
					 pcalibrate9_t pcal_a, pcalibrate9_t pcal_m);

/*int calibration_calc_int(pvector3d_t values_a, pvector3d_t values_m, int k, 
					 pcalibrate9_t pcal_a, pcalibrate9_t pcal_m);*/

/*
	calibration_auto_init()
	
	Инициализация контекста автоматической калибровки.
	
	Return:
	0 - Нет ошибок.
 */
int calibration_auto_init(pcalibrate_cntx_t calc_cntx);

/*
	calibration_auto()
	
	Функция с основной логикой автоматической калибровки.
	Получает на вход текущие измерения с датчиков и производит их усреднение/накопление.
	При накоплении необходимого количества данных производит калибровку.
	
	Return:
	0 - Нет ошибок.
	1 - Ошибка.
	-1 - Есть новая калибровка, требуется прочитать ее из контекста!
 */
int calibration_auto(pcalibrate_cntx_t calc_cntx, vector3df_t *acc, vector3df_t *mag);

/*
	calibration_proc()
	
	Функция наложения калибровки.

	Param:
	
	Return:
	0 - Нет ошибок.
 */
int calibration_proc(vector3df_t *acc_cal, vector3df_t *mag_cal, vector3d_t *acc, vector3d_t *mag, pconfig_t conf);
int calibration_procf(vector3df_t *acc, vector3df_t *mag, pcalibrate_t acc_cal, pcalibrate_t mag_cal);

#endif
