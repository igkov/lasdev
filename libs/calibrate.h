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
	int k;  // ��������� ������� ����������� �������.
	int kc; // ������� ����������� �������.
	int m;  // ������� �������� �� ����������.
	vector3df_t values_a[CAL_MAX_K];
	vector3df_t values_m[CAL_MAX_K];
	calibrate_t cal_a;
	calibrate_t cal_m;
} calibrate_cntx_t, *pcalibrate_cntx_t;

/*
	calibration_next()

	������� ��������� �������������� ��������.
	������ ������� ������������ ��������� ������ �� �������, 
	�������� �������� �������, ��������� ���������� ��������.
	
	Param:
	mag - ��������� �� ��������� ��� ���������� ������ � ������������.
	acc - ��������� �� ��������� ��� ���������� ������ � �������������.
	
	Return:
	0 - ��� ������.
	1 - ������ ��������� ������ �� ��������.
	2 - ������ ��������� ������������� ������, ���������� � ��������!
 */
int calibration_next(vector3df_t *acc, vector3df_t *mag);

/*
	calibration_calc()

	���������� ���������� �� ����������� ��������� � ��������.
	������������ ������� ���������������� ������� ���������.
	
	Param:
	values_a   - ����������� ������ �������� � �����������
	values_m   - ����������� ������ �������� � �����������
	k          - ���������� �������� � �������
	pcal_a     - ��������� � ����������� �������������
	pcal_m     - ��������� � ����������� ������������
	
	Return:
	0 - ��� ������.
	1 - ������, ������� ����� ��������� ��������.
 */
int calibration_calc(pvector3df_t values_a, pvector3df_t values_m, int k, 
					 pcalibrate_t pcal_a, pcalibrate_t pcal_m);


int calibration_calc_int(pvector3d_t values_a, pvector3d_t values_m, int k, 
					 pcalibrate_t pcal_a, pcalibrate_t pcal_m);

/*
	calibration_calc9()

	���������� ���������� �� ����������� ��������� � ��������.
	������������ ������� ���������������� ������� ���������.
	
	Param:
	values_a   - ����������� ������ �������� � �����������
	values_m   - ����������� ������ �������� � �����������
	k          - ���������� �������� � �������
	pcal_a     - ��������� � ����������� �������������
	pcal_m     - ��������� � ����������� ������������
	
	Return:
	0 - ��� ������.
	1 - ������, ������� ����� ��������� ��������.
 */
int calibration_calc9(pvector3df_t values_a, pvector3df_t values_m, int k, 
					 pcalibrate9_t pcal_a, pcalibrate9_t pcal_m);

/*int calibration_calc_int(pvector3d_t values_a, pvector3d_t values_m, int k, 
					 pcalibrate9_t pcal_a, pcalibrate9_t pcal_m);*/

/*
	calibration_auto_init()
	
	������������� ��������� �������������� ����������.
	
	Return:
	0 - ��� ������.
 */
int calibration_auto_init(pcalibrate_cntx_t calc_cntx);

/*
	calibration_auto()
	
	������� � �������� ������� �������������� ����������.
	�������� �� ���� ������� ��������� � �������� � ���������� �� ����������/����������.
	��� ���������� ������������ ���������� ������ ���������� ����������.
	
	Return:
	0 - ��� ������.
	1 - ������.
	-1 - ���� ����� ����������, ��������� ��������� �� �� ���������!
 */
int calibration_auto(pcalibrate_cntx_t calc_cntx, vector3df_t *acc, vector3df_t *mag);

/*
	calibration_proc()
	
	������� ��������� ����������.

	Param:
	
	Return:
	0 - ��� ������.
 */
int calibration_proc(vector3df_t *acc_cal, vector3df_t *mag_cal, vector3d_t *acc, vector3d_t *mag, pconfig_t conf);
int calibration_procf(vector3df_t *acc, vector3df_t *mag, pcalibrate_t acc_cal, pcalibrate_t mag_cal);

#endif
