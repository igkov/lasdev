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
	Преобразование вектора с INT-значениями в вектор с Float-значениями.
 */
void v3_to_v3f(pvector3df_t r, pvector3d_t v);

/*
	v3f_to_q4f()
	Преобразование 3D-вектора в кватернион.
 */
void v3f_to_q4f(pvector4df_t q, pvector3df_t v);

/*
	q4f_to_v3f()
	Преобразование кватерниона в 3D-вектор.
 */
void q4f_to_v3f(pvector3df_t v, pvector4df_t q);

/*
	q4f_to_v3f()
	v1 = v1 + v2;
	Вычисление суммы целочисленных векторов.
 */
void v3_add(pvector3d_t v1, pvector3d_t v2);

/*
	q4f_to_v3f()
	v = v1 * v2;
	Перемножение векторов.
 */
void v3_mul(pvector3d_t v, pvector3d_t v1, pvector3d_t v2);

/*
	v3_sub()
	v1 = v1 - v2;
	Вычисление разности целочисленных векторов.
 */
void v3_sub(pvector3d_t v1, pvector3d_t v2);

/*
	v3_mod()
	Модуль вектора.
 */
int v3_mod(pvector3d_t value);

/*
	v3_mod2()
	Квадрат модуля вектора.
 */
int v3_mod2(pvector3d_t value);

/*
	v3f_norm()
	Нормировка вектора (приведение к единичной длине).
 */
void v3f_norm(pvector3df_t v);

/*
	v3f_inv()
	Инверсия вектора.
 */
void v3f_inv(pvector3df_t v);

/*
	v3f_inv()
	Деление вектора на скаляр.
 */
void v3f_div(pvector3df_t r, pvector3df_t v1, float divider);

/*
	v3f_mod()
	Получение модуля вектора (длины).
 */
float v3f_mod(pvector3df_t v);

/*
	v3f_mod2()
	Получение квадрата модуля вектора (длины).
 */
float v3f_mod2(pvector3df_t v);

/*
	v3f_add()
	Сложение векторов.
	v1 = v1 + v2;
 */
void v3f_add(pvector3df_t v1, pvector3df_t v2);

/*
	v3f_sub()
	Вычитание векторов.
	v1 = v1 - v2;
 */
void v3f_sub(pvector3df_t v1, pvector3df_t v2);

/*
	q4f_norm()
	Нормировка кватерниона (приведение к единичной длине).
 */
void q4f_norm(pvector4df_t q);

/*
	v3f_angle()
	Вычисление угла между 2мя векторами в пространстве.
 */
float v3f_angle(pvector3df_t v1, pvector3df_t v2);

/*
	v3f_angle_norm()
	Вычисление угла между 2мя нормированными векторами в пространстве.
 */
float v3f_angle_norm(pvector3df_t v1, pvector3df_t v2);

/*
	q4f_inv()
	Инвертирование кватерниона.
 */
void q4f_inv(pvector4df_t r, pvector4df_t q);

/*
	v3f_mul()
	Умножение 2х векторов в пространстве.
	v = v1 * v2;
 */
void v3f_mul(pvector3df_t v, pvector3df_t v1, pvector3df_t v2);

/*
	q4f_mul()
	Умножение кватернионов.
	r = q * p;
 */
void q4f_mul(pvector4df_t r, pvector4df_t q, pvector4df_t p);

/*
	transform_g2l()
	Поворот в пространстве вектора v в соотвествии с заданным кватернионом pstate.
	Результат преобразования сохраняется в векторе r.
 */
void transform_g2l(pvector3df_t r, pvector4df_t pstate, pvector3df_t v);

/*
	q4f_create_rotate()
	Создание поворота в пространстве от вектора from к вектору to.
	Функция вычисляет кватернион, соответвующий такому повороту.
 */
void q4f_create_rotate(pvector4df_t r, pvector3df_t from, pvector3df_t to);

/*
	axes_mesh_f();
	Перестановка осей у вектора vector в соответсвии с таблице в mesh.
 */
int axes_mesh_f(pvector3df_t vector, pvector3d_t mesh);


#endif // __VECTOR_H__
