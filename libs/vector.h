#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdint.h>

// v3 - 3d integer vector
// v3f - 3d float vector
// v4 - 4d integer vector
// v4f - 4d float point vector
// q4f - float point quaternion
// q4 - integer quaternion

#define ABS(a)   ((a)>0?(a):-(a))
#define SIGN(a)  ((a)>0?(1):(-1))
#define SQR(a)   ((a)*(a))
#define PI       3.1415926535f
#define SIG(a)   ((a)>0?1:0)


typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
} vector3d_t, *pvector3d_t;

typedef struct
{
	float x;
	float y;
	float z;
} vector3df_t, *pvector3df_t;

typedef struct
{
	float a1;
	float a2;
	float a3;
	float a4;
} vector4df_t, *pvector4df_t;

typedef struct
{
	vector3df_t koeff;
	vector3df_t offset;
} calibrate_t, *pcalibrate_t;

typedef struct
{
	float x0;
	float y0;
	float z0;
	float a;
	float b;
	float c;
	float d;
	float e;
	float f;
} calibrate9_t, *pcalibrate9_t;

/*
	v3_to_v3f()
	�������������� ������� � INT-���������� � ������ � Float-����������.
 */
void v3_to_v3f(pvector3df_t r, pvector3d_t v);

/*
	v3f_to_q4f()
	�������������� 3D-������� � ����������.
 */
void v3f_to_q4f(pvector4df_t q, pvector3df_t v);

/*
	q4f_to_v3f()
	�������������� ����������� � 3D-������.
 */
void q4f_to_v3f(pvector3df_t v, pvector4df_t q);

/*
	q4f_to_v3f()
	v1 = v1 + v2;
	���������� ����� ������������� ��������.
 */
void v3_add(pvector3d_t v1, pvector3d_t v2);

/*
	q4f_to_v3f()
	v = v1 * v2;
	������������ ��������.
 */
void v3_mul(pvector3d_t v, pvector3d_t v1, pvector3d_t v2);

/*
	v3_sub()
	v1 = v1 - v2;
	���������� �������� ������������� ��������.
 */
void v3_sub(pvector3d_t v1, pvector3d_t v2);

/*
	v3_mod()
	������ �������.
 */
int v3_mod(pvector3d_t value);

/*
	v3_mod2()
	������� ������ �������.
 */
int v3_mod2(pvector3d_t value);

/*
	v3f_norm()
	���������� ������� (���������� � ��������� �����).
 */
void v3f_norm(pvector3df_t v);

/*
	v3f_inv()
	�������� �������.
 */
void v3f_inv(pvector3df_t v);

/*
	v3f_inv()
	������� ������� �� ������.
 */
void v3f_div(pvector3df_t r, pvector3df_t v1, float divider);

/*
	v3f_mod()
	��������� ������ ������� (�����).
 */
float v3f_mod(pvector3df_t v);

/*
	v3f_mod2()
	��������� �������� ������ ������� (�����).
 */
float v3f_mod2(pvector3df_t v);

/*
	v3f_add()
	�������� ��������.
	v1 = v1 + v2;
 */
void v3f_add(pvector3df_t v1, pvector3df_t v2);

/*
	v3f_sub()
	��������� ��������.
	v1 = v1 - v2;
 */
void v3f_sub(pvector3df_t v1, pvector3df_t v2);

/*
	q4f_norm()
	���������� ����������� (���������� � ��������� �����).
 */
void q4f_norm(pvector4df_t q);

/*
	v3f_angle()
	���������� ���� ����� 2�� ��������� � ������������.
 */
float v3f_angle(pvector3df_t v1, pvector3df_t v2);

/*
	v3f_angle_norm()
	���������� ���� ����� 2�� �������������� ��������� � ������������.
 */
float v3f_angle_norm(pvector3df_t v1, pvector3df_t v2);

/*
	q4f_inv()
	�������������� �����������.
 */
void q4f_inv(pvector4df_t r, pvector4df_t q);

/*
	v3f_mul()
	��������� 2� �������� � ������������.
	v = v1 * v2;
 */
void v3f_mul(pvector3df_t v, pvector3df_t v1, pvector3df_t v2);

/*
	q4f_mul()
	��������� ������������.
	r = q * p;
 */
void q4f_mul(pvector4df_t r, pvector4df_t q, pvector4df_t p);

/*
	transform_g2l()
	������� � ������������ ������� v � ����������� � �������� ������������ pstate.
	��������� �������������� ����������� � ������� r.
 */
void transform_g2l(pvector3df_t r, pvector4df_t pstate, pvector3df_t v);

/*
	q4f_create_rotate()
	�������� �������� � ������������ �� ������� from � ������� to.
	������� ��������� ����������, ������������� ������ ��������.
 */
void q4f_create_rotate(pvector4df_t r, pvector3df_t from, pvector3df_t to);

/*
	axes_mesh_f();
	������������ ���� � ������� vector � ����������� � ������� � mesh.
 */
int axes_mesh_f(pvector3df_t vector, pvector3d_t mesh);


#endif // __VECTOR_H__
