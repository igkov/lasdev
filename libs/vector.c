#include <math.h>
#include "vector.h"

void v3_to_v3f(pvector3df_t r, pvector3d_t v)
{
	r->x = v->x;
	r->y = v->y;
	r->z = v->z;
}

void v3f_to_q4f(pvector4df_t q, pvector3df_t v)
{
	q->a1 = 0;
	q->a2 = v->x;
	q->a3 = v->y;
	q->a4 = v->z;
}

void q4f_to_v3f(pvector3df_t v, pvector4df_t q)
{
	v->x = q->a2;
	v->y = q->a3;
	v->z = q->a4;
}

void v3_add(pvector3d_t v1, pvector3d_t v2)
{
	v1->x += v2->x;
	v1->y += v2->y;
	v1->z += v2->z;
}

void v3_sub(pvector3d_t v1, pvector3d_t v2)
{
	v1->x -= v2->x;
	v1->y -= v2->y;
	v1->z -= v2->z;
}

void v3_mul(pvector3d_t v, pvector3d_t v1, pvector3d_t v2)
{
	v->x = v1->y * v2->z - v1->z * v2->y;
	v->y = v1->z * v2->x - v1->x * v2->z;
	v->z = v1->x * v2->y - v1->y * v2->x;
}

int v3_mod(pvector3d_t value)
{
#if 1
	return (int)sqrtf(SQR(value->x) + SQR(value->y) + SQR(value->z));
#else
	int ret;
	int tmp;
	int i;

	tmp = SQR(value->x) + SQR(value->y) + SQR(value->z);
	ret = tmp>>1;

	// Рекурсивное вычисление корня:
	for (i = 0; i < 5; i++)
	{
		ret = ret + tmp / ret;
	}

	return ret;
#endif
}

int v3_mod2(pvector3d_t value)
{
	return SQR(value->x) + SQR(value->y) + SQR(value->z);
}

void v3f_add(pvector3df_t v1, pvector3df_t v2)
{
	v1->x += v2->x;
	v1->y += v2->y;
	v1->z += v2->z;
}

void v3f_sub(pvector3df_t v1, pvector3df_t v2)
{
	v1->x -= v2->x;
	v1->y -= v2->y;
	v1->z -= v2->z;
}

void v3f_norm(pvector3df_t v)
{
	float n;
	n = sqrtf((SQR(v->x) + SQR(v->y) + SQR(v->z)));
	v->x /= n;
	v->y /= n;
	v->z /= n;
}

void v3f_inv(pvector3df_t v)
{
	v->x = -v->x;
	v->y = -v->y;
	v->z = -v->z;
}

void v3f_div(pvector3df_t r, pvector3df_t v1, float divider)
{
	r->x = v1->x / divider;
	r->y = v1->y / divider;
	r->z = v1->z / divider;
}

void q4f_norm(pvector4df_t q)
{
	float n;
	n = sqrtf((SQR(q->a1) + SQR(q->a2) + SQR(q->a3) + SQR(q->a4)));
	q->a1 /= n;
	q->a2 /= n;
	q->a3 /= n;
	q->a4 /= n;
}

/*
	2D: cos a = (x1*x2 + y1*y2) / sqrt ((x1*x1 + y1*y1) * (x2*x2 + y2*y2))
	3D: cos a = (x1*x2 + y1*y2 + z1*z2) / sqrt ((x1*x1 + y1*y1 + z1*z1) * (x2*x2 + y2*y2 + z2*z2))
 */
float v3f_angle(pvector3df_t v1, pvector3df_t v2)
{
	float ret;
	ret = (v1->x * v2->x  + v1->y * v2->y + v1->z * v2->z);
	ret /= sqrtf((SQR(v1->x) + SQR(v1->y) + SQR(v1->z))*
				(SQR(v2->x) + SQR(v2->y) + SQR(v2->z)));
	ret = acosf(ret);
	return ret;
}

float v3f_angle_norm(pvector3df_t v1, pvector3df_t v2)
{
	float ret;
	ret = (v1->x * v2->x  + v1->y * v2->y + v1->z * v2->z);
	ret = acosf(ret);
	return ret;
}

void q4f_inv(pvector4df_t r, pvector4df_t q)
{
	r->a1 =  q->a1;
	r->a2 = -q->a2;
	r->a3 = -q->a3;
	r->a4 = -q->a4;
}

void v3f_mul(pvector3df_t v, pvector3df_t v1, pvector3df_t v2)
{
	v->x = v1->y * v2->z - v1->z * v2->y;
	v->y = v1->z * v2->x - v1->x * v2->z;
	v->z = v1->x * v2->y - v1->y * v2->x;
}

void q4f_mul(pvector4df_t r, pvector4df_t q, pvector4df_t p)
{
	r->a1 = q->a1 * p->a1 - q->a2 * p->a2 - q->a3 * p->a3 - q->a4 * p->a4;
	r->a2 = q->a2 * p->a1 + q->a1 * p->a2 - q->a4 * p->a3 + q->a3 * p->a4;
	r->a3 = q->a3 * p->a1 + q->a4 * p->a2 + q->a1 * p->a3 - q->a2 * p->a4;
	r->a4 = q->a4 * p->a1 - q->a3 * p->a2 + q->a2 * p->a3 + q->a1 * p->a4;
}

float v3f_mod(pvector3df_t v)
{
	float ret;
	ret = sqrtf(SQR(v->x) + SQR(v->y) + SQR(v->z));
	return ret;
}

float v3f_mod2(pvector3df_t v)
{
	float ret;
	ret = SQR(v->x) + SQR(v->y) + SQR(v->z);
	return ret;
}

void transform_g2l(pvector3df_t r, pvector4df_t pstate, pvector3df_t v)
{
	vector4df_t v4;
	vector4df_t qi;
	vector4df_t t;
	
	// v3 to q4
	v3f_to_q4f(&v4, v);
	// q * v
	q4f_mul(&t, pstate, &v4);
	// q'
	q4f_inv(&qi, pstate);
	// (q * v) * q'
	q4f_mul(&v4, &t, &qi);
	// q4 to v3
	q4f_to_v3f(r, &v4);
}

void transform_l2g(pvector3df_t r, pvector4df_t pstate, pvector3df_t v)
{
	vector4df_t v4;
	vector4df_t qi;
	vector4df_t t;
	
	// v3 to q4
	v3f_to_q4f(&v4, v);
	// v * q
	q4f_mul(&t, &v4, pstate);
	// q'
	q4f_inv(&qi, pstate);
	// q' * (q * v)
	q4f_mul(&v4, &qi, &t);
	// q4 to v3
	q4f_to_v3f(r, &v4);
}

void q4f_create_rotate(pvector4df_t r, pvector3df_t from, pvector3df_t to)
{
	vector3df_t t;
	float a;
	// Собираем поворот:
	v3f_mul(&t, from, to);
	v3f_norm(&t);
	a = v3f_angle(from, to);
	r->a1 = cosf(a/2);
	r->a2 = t.x * sinf(a/2);
	r->a3 = t.y * sinf(a/2);
	r->a4 = t.z * sinf(a/2);
}

int axes_mesh_f(pvector3df_t vector, pvector3d_t mesh)
{
	vector3df_t new;

	if (ABS(mesh->x) > 3 || ABS(mesh->y) > 3 || ABS(mesh->z) > 3)
	{
		return 1;
	}

	new.x = ((float*)vector)[ABS(mesh->x)-1] * SIGN(mesh->x);
	new.y = ((float*)vector)[ABS(mesh->y)-1] * SIGN(mesh->y);
	new.z = ((float*)vector)[ABS(mesh->z)-1] * SIGN(mesh->z);

	*vector = new;

	return 0;
}
