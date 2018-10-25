#if !defined( CORTEX )
#include <stdio.h>
#include <windows.h>
#endif
#include <stdint.h>

#include "io.h"
#include "vector.h"
#include "solve.h"
#if !defined( CORTEX )
#include "solve9.h"
#endif
#include "calibrate.h"
#include "lowpass.h"

#if defined( CORTEX )
#include "type.h"
#include "event.h"
#include "lsm303.h"
#elif defined( USB )
int get_raw_data(int op, pvector3d_t data);
#elif defined( BT )
int get_raw_data(pvector3d_t data_a, pvector3d_t data_m);
#endif

/*
	Стандартная процедура калибровки.
	Функция для последовательной калибровки по требованию.
 */
int calibration_next(vector3df_t *acc, vector3df_t *mag)
{
	int i = 0;
	int rep = 0;
	
	vector3df_t mean_m;
	vector3df_t mean_a;

	// mag
	while (i < CAL_MEAN_K)
	{
		int ret;
		vector3d_t data_m;
		vector3d_t data_a;
		vector3df_t mean_t, data_t;
		
#if defined( CORTEX )
		ret = lsm303_get_acc(&data_a);
#else
	#if defined( USB )
		ret = get_raw_data(OP_GET_LSM_ACC, &data_a);
	#elif defined( BT )
		ret = get_raw_data(&data_a, &data_a);
	#endif
#endif
		if (ret)
		{
			return 1;
		}
		// Копируем вектор в данные:
		v3_to_v3f(&data_t, &data_a);
		if (i > 0)
		{
			// Усредняем накопленные данные:
			v3f_div(&mean_t, &mean_a, (float)i);
			// Определяем разность нового вектора и усредненного:
			v3f_sub(&data_t, &mean_t);
			// Если модуль разности более 5%, считаем, что устройство движется
			if ((v3f_mod(&data_t) / v3f_mod(&mean_t)) > 0.05f)
			{
				// Превышено максимальное число попыток получения калибровочного значения!
				if (rep++ > CAL_MAX_REPEATE)
				{
					return 2;
				}
				// Если начальное значение, тогда перезаписываем накопление:
				i = 0;
#if defined( CORTEX )
				delay_ms(15);
#else
				Sleep(10);
#endif
				continue;
			}
			
			// Копируем вектор в данные (т.к. были перезатерты):
			v3_to_v3f(&data_t, &data_a);
			// Добавляем в усреднение:
			v3f_add(&mean_a, &data_t);
		}
		else
		{
			// Добавляем в усреднение:
			mean_a = data_t;
		}
		
#if defined( CORTEX )
		ret = lsm303_get_mag(&data_m);
#else
	#if defined( USB )
		ret = get_raw_data(OP_GET_LSM_MAG, &data_m);
	#endif
#endif
		if (ret)
		{
			return 1;
		}
		// Копируем вектор в данные:
		v3_to_v3f(&data_t, &data_m);
		if (i > 0)
		{
			// Добавляем в усреднение:
			v3f_add(&mean_m, &data_t);
		}
		else
		{
			mean_m = data_t;
		}
		
#if defined( CORTEX )
		delay_ms(5);
#else
		Sleep(1);
#endif
		// Приращиваем только если все в порядке:
		i++;
	}

	// Усредняем накопленные данные:
	v3f_div(acc, &mean_a, (float)i);
	v3f_div(mag, &mean_m, (float)i);

	// Возврат, все в порядке:
	return 0;
}

/*
	Вычисление калибровки
 */
int calibration_calc(pvector3df_t values_a, pvector3df_t values_m, int k, 
					 pcalibrate_t pcal_a, pcalibrate_t pcal_m)
{
	if (k > CAL_MAX_K)
	{
		return 1;
	}
	
	if (k < 6)
	{
		return 1;
	}
	
	// Решаем переопределенную систему линейных уравнений:
	solve_fill_float(values_m, k);
	solve_linear_system(pcal_m, k);
	solve_fill_float(values_a, k);
	solve_linear_system(pcal_a, k);

#if ( DEVELOPER_BUILD == 1 )
	printf("Calibrated!\r\n");
	printf("Magnetometer values:\r\n\tOffset = { %.4ff, %.4ff, %.4ff }\r\n\tKoeff = { %.4ff, %.4ff, %.4ff }\r\n",
		pcal_m->offset.x, pcal_m->offset.y, pcal_m->offset.z, pcal_m->koeff.x, pcal_m->koeff.y, pcal_m->koeff.z);
	printf("Accelerometer values:\r\n\tOffset = { %.4ff, %.4ff, %.4ff }\r\n\tKoeff = { %.4ff, %.4ff, %.4ff }\r\n",
		pcal_a->offset.x, pcal_a->offset.y, pcal_a->offset.z, pcal_a->koeff.x, pcal_a->koeff.y, pcal_a->koeff.z);
#endif

	return 0;
}

/*
	Вычисление калибровки
	(увеличено кол-во целочисленных вычислений, уменьшены требования к памяти)
 */
int calibration_calc_int(pvector3d_t values_a, pvector3d_t values_m, int k, 
					 pcalibrate_t pcal_a, pcalibrate_t pcal_m)
{
	if (k > CAL_MAX_K)
	{
		return 1;
	}
	
	if (k < 6)
	{
		return 1;
	}
	
	// Решаем переопределенную систему линейных уравнений:
	solve_fill_int(values_m, k);
	solve_linear_system(pcal_m, k);
	solve_fill_int(values_a, k);
	solve_linear_system(pcal_a, k);
	
	//printf("Calibrated!\r\n");
	//printf("Magnetometer values:\r\n  Offset = { %.4ff, %.4ff, %.4ff }\r\n  Koeff = { %.4ff, %.4ff, %.4ff }\r\n",
	//	pcal_m->offset.x, pcal_m->offset.y, pcal_m->offset.z, pcal_m->koeff.x, pcal_m->koeff.y, pcal_m->koeff.z);
	//printf("Accelerometer values:\r\n  Offset = { %.4ff, %.4ff, %.4ff }\r\n  Koeff = { %.4ff, %.4ff, %.4ff }\r\n",
	//	pcal_a->offset.x, pcal_a->offset.y, pcal_a->offset.z, pcal_a->koeff.x, pcal_a->koeff.y, pcal_a->koeff.z);
	
	return 0;
}

#if !defined( CORTEX  )
/*
	Вычисление калибровки
 */
int calibration_calc9(pvector3df_t values_a, pvector3df_t values_m, int k, 
					 pcalibrate9_t pcal_a, pcalibrate9_t pcal_m)
{
	if (k > CAL_MAX_K)
	{
		return 1;
	}
	
	if (k < 9)
	{
		return 1;
	}
	
	//// Решаем переопределенную систему линейных уравнений:
	solve_fill_float9(values_m, k);
	solve_linear_system9(pcal_m, k);
	solve_fill_float9(values_a, k);
	solve_linear_system9(pcal_a, k);
	
#if ( DEVELOPER_BUILD == 1 )
	printf("Calibrated!\r\n");
	printf("Magnetometer values:\r\n\tOffset = { %.4ff, %.4ff, %.4ff }\r\n\ta,b,c,d,e,f = { %.4ff, %.4ff, %.4ff, %.4ff, %.4ff, %.4ff }\r\n",
		pcal_m->x0, pcal_m->y0, pcal_m->z0, pcal_m->a, pcal_m->b, pcal_m->c, pcal_m->d, pcal_m->e, pcal_m->f);
	printf("Accelerometer values:\r\n\tOffset = { %.4ff, %.4ff, %.4ff }\r\n\ta,b,c,d,e,f = { %.4ff, %.4ff, %.4ff, %.4ff, %.4ff, %.4ff }\r\n",
		pcal_a->x0, pcal_a->y0, pcal_a->z0, pcal_a->a, pcal_a->b, pcal_a->c, pcal_a->d, pcal_a->e, pcal_a->f);
#endif

	return 0;
}
#endif

int calibration_auto_init(pcalibrate_cntx_t calc_cntx)
{
	int i;

	// Нет значений:
	calc_cntx->k = 0;
	calc_cntx->kc = 0;
	calc_cntx->m = 0;
	// Обнуляем калибровку акселерометра:
	calc_cntx->cal_a.koeff.x  = 1.0f;
	calc_cntx->cal_a.koeff.y  = 1.0f;
	calc_cntx->cal_a.koeff.z  = 1.0f;
	calc_cntx->cal_a.offset.x = 0.0f;
	calc_cntx->cal_a.offset.y = 0.0f;
	calc_cntx->cal_a.offset.z = 0.0f;
	// Обнуляем калибровку магнетометра:
	calc_cntx->cal_m.koeff.x  = 1.0f;
	calc_cntx->cal_m.koeff.y  = 1.0f;
	calc_cntx->cal_m.koeff.z  = 1.0f;
	calc_cntx->cal_m.offset.x = 0.0f;
	calc_cntx->cal_m.offset.y = 0.0f;
	calc_cntx->cal_m.offset.z = 0.0f;
	// Обнуляем массивы калибровочных выборок:
	for (i=0; i<CAL_MAX_K; i++)
	{
		calc_cntx->values_a[i].x = 0.0f;
		calc_cntx->values_a[i].y = 0.0f;
		calc_cntx->values_a[i].z = 0.0f;
		calc_cntx->values_m[i].x = 0.0f;
		calc_cntx->values_m[i].y = 0.0f;
		calc_cntx->values_m[i].z = 0.0f;
	}
	return 0;
}

int calibration_auto(pcalibrate_cntx_t calc_cntx, vector3df_t *acc, vector3df_t *mag)
{
	int i;
	int f;
	int ret;
	
	// Функция вызывается для каждой выборки из датчиков.
	
	// 1. Цикл накопления измерений.
	// Проверка, что положение не менялось.
	// Накопление.
	// Усреднение.
	
	#define DELTA_AUTO 0.03f
	
	if (calc_cntx->m == 0)
	{
		calc_cntx->values_a[calc_cntx->k] = *acc;
		calc_cntx->values_m[calc_cntx->k] = *mag;
		
		calc_cntx->m++;
		
		return 0;
	}
	else
	{
		vector3df_t delta1, delta2;
		delta1 = *acc;
		v3f_sub(&delta1, &(calc_cntx->values_a[calc_cntx->k]));
		delta2 = *mag;
		v3f_sub(&delta2, &(calc_cntx->values_m[calc_cntx->k]));
		
		if (calc_cntx->m == CAL_MEAN_K)
		{
			printf("[1] Fix position!\r\n");
		}
		
		if ((v3f_mod2(&delta1) / v3f_mod2(acc)) < DELTA_AUTO &&
			(v3f_mod2(&delta2) / v3f_mod2(mag)) < DELTA_AUTO)
		{
			calc_cntx->values_a[calc_cntx->k].x = lowpass(acc->x, CALIBRATE_LOWPASS_A, calc_cntx->values_a[calc_cntx->k].x);
			calc_cntx->values_a[calc_cntx->k].y = lowpass(acc->y, CALIBRATE_LOWPASS_A, calc_cntx->values_a[calc_cntx->k].y);
			calc_cntx->values_a[calc_cntx->k].z = lowpass(acc->z, CALIBRATE_LOWPASS_A, calc_cntx->values_a[calc_cntx->k].z);
			
			calc_cntx->values_m[calc_cntx->k].x = lowpass(mag->x, CALIBRATE_LOWPASS_A, calc_cntx->values_m[calc_cntx->k].x);
			calc_cntx->values_m[calc_cntx->k].y = lowpass(mag->y, CALIBRATE_LOWPASS_A, calc_cntx->values_m[calc_cntx->k].y);
			calc_cntx->values_m[calc_cntx->k].z = lowpass(mag->z, CALIBRATE_LOWPASS_A, calc_cntx->values_m[calc_cntx->k].z);
			
			calc_cntx->m++;
			
			//printf("[1] m = %d\r\n", calc_cntx->m);
			
			// Значение добавлено.
			return 0;
		}
		else
		{
			if (calc_cntx->m > CAL_MEAN_K)
			{
				// Сбрасываем информацию об этом:
				calc_cntx->m = 0;
				// Есть усреднение, продолжаем работу функции.
			}
			else
			{
				// Не найдено нужное кол-во значений:
				calc_cntx->values_a[calc_cntx->k] = *acc;
				calc_cntx->values_m[calc_cntx->k] = *mag;
				
				calc_cntx->m = 1;
				
				return 0;
			}
		}
	}
	
	printf("[2] kc = %d\r\n", calc_cntx->kc);

	//printf("ACCN: (%f, %f, %f)\r\n", calc_cntx->values_a[calc_cntx->k].x, calc_cntx->values_a[calc_cntx->k].y, calc_cntx->values_a[calc_cntx->k].z);
	//printf("MAGN: (%f, %f, %f)\r\n", calc_cntx->values_m[calc_cntx->k].x, calc_cntx->values_m[calc_cntx->k].y, calc_cntx->values_m[calc_cntx->k].z);
	//printf("ACC0: (%f, %f, %f)\r\n", calc_cntx->values_a[0].x, calc_cntx->values_a[0].y, calc_cntx->values_a[0].z);
	//printf("MAG0: (%f, %f, %f)\r\n", calc_cntx->values_m[0].x, calc_cntx->values_m[0].y, calc_cntx->values_m[0].z);
	
	// 2. Полученно среднее.
	// Можно пробовать добавить в массив калибровки.
	// Проход по присутствующим векторам, чтобы понять что нет совпадения по углу менее 15 градусов.
	
	for (i = 0; i < calc_cntx->kc; i++)
	{
		if (i != calc_cntx->k)
		{
			float ang1, ang2;
			ang1 = v3f_angle(&(calc_cntx->values_a[calc_cntx->k]), &(calc_cntx->values_a[i]));
			ang2 = v3f_angle(&(calc_cntx->values_m[calc_cntx->k]), &(calc_cntx->values_m[i]));
			if ((ang1 < MIN_ANGLE) && (ang2 < MIN_ANGLE))
			{
				printf("[3] not save 1 (ang1 = %f, ang2 = %f)!\r\n", ang1, ang2);
				// Отбрасываем вектор!
				return 0;
			}
		}
	}
	
	// Проверка прошла, добавляем в список:
	if (calc_cntx->k == calc_cntx->kc)
	{
		calc_cntx->k++;
		calc_cntx->kc++;
	}
	
	printf("[4] add ok!\r\n");
	
	if (calc_cntx->kc < CAL_MIN_K)
		return 0;
	
	printf("[5] list full!\r\n");

	// 3. Накоплено нужное количество векторов.
	// Проверка, что все 8 сегментов сферы охватываются значениями калибровки:
	// ((x, y, z), (x, y, -z), (x, -y, z), (x, -y, -z), (-x, y, z), (-x, y, -z), (-x, -y, z), (-x, -y, -z))
	
	f = 0;
	for (i = 0; i < calc_cntx->kc; i++)
	{
		int t;
		t = ((SIG(calc_cntx->values_a[i].x) << 2) + 
			 (SIG(calc_cntx->values_a[i].y) << 1) + 
			 (SIG(calc_cntx->values_a[i].z) << 0));
		
		if ((1<<t)&f)
		{
			// Уже установлен!
		}
		
		// Добавляем флаг:
		f |= 1 << t;
	}
	
	printf("[6] check...\r\n");
	
	if (f != 0x00FF) 
	{
		if (calc_cntx->kc < CAL_MAX_K)
		{
			return 0;
		}
	}
	
	printf("[5] ok...\r\n");

	// 4. Если не охватываются, удаление из массива 2-3 элементов имеющих наименьший угол с остальными векторами (<30 град).
	
	if (f != 0x00FF) 
	{
		// Требуется добирать полусферы!!!
		printf("[6] f = %04x..\r\n", f);
		
		//calc_cntx->k = ...;
		
		return 0;
	}
	
	printf("[7] calibration_calc()...\r\n");
	// 5. Запуск на калибровку
	ret = calibration_calc(calc_cntx->values_a, calc_cntx->values_m, calc_cntx->k, 
					 &calc_cntx->cal_a, &calc_cntx->cal_m);
	if (ret)
	{
		printf("calibration_calc(%d) return %d\r\n", calc_cntx->k, ret);
		return 0;
	}

	// 6. Калибровано!
	return 1;
}

int calibration_proc(vector3df_t *acc_cal, vector3df_t *mag_cal, vector3d_t *acc, vector3d_t *mag, pconfig_t conf)
{
	if (acc != NULL)
	{
		acc_cal->x = ((float)acc->x - conf->acc_cal.offset.x) / conf->acc_cal.koeff.x;
		acc_cal->y = ((float)acc->y - conf->acc_cal.offset.y) / conf->acc_cal.koeff.y;
		acc_cal->z = ((float)acc->z - conf->acc_cal.offset.z) / conf->acc_cal.koeff.z;
		
		axes_mesh_f(acc_cal, &(conf->acc_shuff));
	}
	
	if (mag != NULL)
	{
		mag_cal->x = ((float)mag->x - conf->mag_cal.offset.x) / conf->mag_cal.koeff.x;
		mag_cal->y = ((float)mag->y - conf->mag_cal.offset.y) / conf->mag_cal.koeff.y;
		mag_cal->z = ((float)mag->z - conf->mag_cal.offset.z) / conf->mag_cal.koeff.z;
		
		axes_mesh_f(mag_cal, &(conf->mag_shuff));
	}
	
	return 0;
}

int calibration_procf(vector3df_t *acc, vector3df_t *mag, pcalibrate_t acc_cal, pcalibrate_t mag_cal)
{
	if (acc != NULL)
	{
		acc->x = (acc->x - acc_cal->offset.x) / acc_cal->koeff.x;
		acc->y = (acc->y - acc_cal->offset.y) / acc_cal->koeff.y;
		acc->z = (acc->z - acc_cal->offset.z) / acc_cal->koeff.z;
		
		//axes_mesh_f(acc, &(acc_shuff));
	}
	
	if (mag != NULL)
	{
		mag->x = (mag->x - mag_cal->offset.x) / mag_cal->koeff.x;
		mag->y = (mag->y - mag_cal->offset.y) / mag_cal->koeff.y;
		mag->z = (mag->z - mag_cal->offset.z) / mag_cal->koeff.z;
		
		//axes_mesh_f(mag, &(mag_shuff));
	}
	
	return 0;
}
