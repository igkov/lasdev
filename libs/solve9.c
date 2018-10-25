#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <math.h>

#include "solve9.h"

float matrix_source[9*MAX_K];
float matrix_x2[MAX_K];
float matrix_trans[9*MAX_K];

#define SQR(a) ((a)*(a))
#define SWAP(a, b, t) {t temp;t=a;a=b;b=t}

static int transpose(float *src, int m, int n, float *dest)
{
	int i;
	int pos;

	for (i=0; i<m*n; i++)
	{
		pos = (i/m) + (i%m)*n;
		dest[pos] = src[i];
	}
	return 0;
}

static int inverse9(float *data)
{
	int gaus_ostatok;
	float gaus_deter,gaus_minved;

	//   Based on realization:
	//  (c) Drobotenko    http://drobotenko.com

	int rn[9],cn[9];
	int j,k;

	for (j=9;j--;)
		rn[j]=cn[j]=j;

#if defined( CORTEX )
	gaus_minved=FLT_MAX/10;
#else
	gaus_minved=1e99;
#endif
	gaus_deter=1;

	for(gaus_ostatok=9;gaus_ostatok;gaus_ostatok--)
	{
		int jved=0,kved=0;
		int jt,kt;
		float vved=-1;
		float t;

		// поиск ведущего
		for(j=9;j--;)
			if(~rn[j])
				for(k=9;k--;)
					if(~cn[k])
						if(vved<fabsf(data[j+9*k]))
						{
							vved = fabsf(data[j+9*k]);
							jved = j;
							kved = k;
						}

		if(gaus_minved>vved)
			gaus_minved=vved;

		gaus_deter*=data[jved+9*kved];

#if defined( CORTEX )
		if(vved < (FLT_MIN*10))
#else
		if(vved < 1e-99)
#endif
		{
			for(j=9;j--;)
			{
				if(~rn[j])
					for(k=9;k--;)
						data[j+9*k]=NAN;
				if(~cn[j])
					for(k=9;k--;)
						data[k+9*j]=NAN;
			}
			return 1;
		}

		jt=rn[jved];
		kt=cn[kved];

		// перестановки
		for(j=9;j--;)
		{
			t=data[kt+9*j];
			data[kt+9*j]=data[jved+9*j];
			data[jved+9*j]=t;
		}

		for(j=9;j--;)
		{
			t=data[j+9*jt];
			data[j+9*jt]=data[j+9*kved];
			data[j+9*kved]=t;
		}

		rn[jved]=rn[kt];
		cn[kved]=cn[jt];
		rn[kt]=cn[jt]=-1;
		vved=data[kt+9*jt];
		data[kt+9*jt]=1;

		for(j=9;j--;)
		{
			float mul;
			if(j==kt)
				continue;
			mul=data[j+9*jt]/vved;
			data[j+9*jt]=0;
			for(k=9;k--;)
				data[j+9*k]-=data[kt+9*k]*mul;
		}

		for(k=9;k--;)
			data[kt+9*k]/=vved;
	}
	
	return 0;
}

static int multiply(float *a, int ax, int ay, float *b, int bx, int by, float *res)
{
	int i,j,k;
	if (ax!=by)
		return 1;
	for (i=0;i<ay;i++)
		for (j=0;j<bx;j++)
		{
			float sum=0;
			for (k=0;k<ax;k++)
			{
				sum+=a[ax*i+k]*b[bx*k+j];
			}
			res[i*bx+j]=sum;
		}
	return 0;
}

int solve_fill_int9(pvector3d_t values, int k)
{
	int i;
	
	for (i = 0; i < k; i++)
	{
		matrix_source[i*9+0] = (float)values[i].x;
		matrix_source[i*9+1] = (float)values[i].y;
		matrix_source[i*9+2] = (float)values[i].z;
		matrix_source[i*9+3] = (float)-SQR(values[i].y);
		matrix_source[i*9+4] = (float)-SQR(values[i].z);
		matrix_source[i*9+5] = (float)values[i].x*values[i].y;
		matrix_source[i*9+6] = (float)values[i].x*values[i].z;
		matrix_source[i*9+7] = (float)values[i].y*values[i].z;
		matrix_source[i*9+8] = 1;
		matrix_x2[i]         = (float)SQR(values[i].x);
	}
	return 0;
}

int solve_fill_float9(pvector3df_t values, int k)
{
	int i;
	
	for (i = 0; i < k; i++)
	{
		matrix_source[i*9+0] = (float)values[i].x;
		matrix_source[i*9+1] = (float)values[i].y;
		matrix_source[i*9+2] = (float)values[i].z;
		matrix_source[i*9+3] = (float)-SQR(values[i].y);
		matrix_source[i*9+4] = (float)-SQR(values[i].z);
		matrix_source[i*9+5] = (float)-values[i].x*values[i].y;
		matrix_source[i*9+6] = (float)-values[i].x*values[i].z;
		matrix_source[i*9+7] = (float)-values[i].y*values[i].z;
		matrix_source[i*9+8] = 1;
		matrix_x2[i]         = (float)SQR(values[i].x);
	}
	return 0;
}

#define R_1 res[0]
#define R_2 res[1]
#define R_3 res[2]
#define R_4 res[3]
#define R_5 res[4]
#define R_6 res[5]
#define R_7 res[6]
#define R_8 res[7]
#define R_9 res[8]

int solve_linear_system9(pcalibrate9_t cal, int k)
{
	int ret;
	float res[9];
	float matrix_forinv[9*9];

	// Решаем систему:

	// 1. Ищем псевообратное: A+ = (A* A)^-1 A*:
	ret = transpose(matrix_source, 9, k, matrix_trans);
	if (ret) return ret;
	multiply(matrix_trans, k, 9, matrix_source, 9, k, matrix_forinv);
	inverse9(matrix_forinv);

	// 2. Непосредственно решаем систему:
	multiply(matrix_forinv, 9, 9, matrix_trans, k, 9, matrix_source);
	multiply(matrix_source, k, 9, matrix_x2, 1, k, res);
	
	// 3. Сохраняем коэффициенты:
	cal->x0 =  1/2*(-4*R_1*R_5*R_4+R_1*SQR(R_8)+2*R_6*R_2*R_5-R_6*R_8*R_3+2*R_7*R_3*R_4-R_7*R_8*R_2)/(-4*R_5*R_4+R_5*SQR(R_6)-R_6*R_7*R_8+SQR(R_7)*R_4+SQR(R_8));
	cal->y0 =  1/2*(-4*R_2*R_5+   R_2*SQR(R_7)+2*R_6*R_1*R_5-R_6*R_7*R_3+2*R_8*R_3-R_8*R_7*R_1)   /(-4*R_5*R_4+R_5*SQR(R_6)-R_6*R_7*R_8+SQR(R_7)*R_4+SQR(R_8));
	cal->z0 = -1/2*(4*R_3*R_4-    R_3*SQR(R_6)-2*R_7*R_1*R_4+R_6*R_7*R_2-2*R_8*R_2+R_8*R_6*R_1)   /(-4*R_5*R_4+R_5*SQR(R_6)-R_6*R_7*R_8+SQR(R_7)*R_4+SQR(R_8));
	cal->a  = sqrtf(R_9+SQR(cal->x0)+R_4*SQR(cal->y0)+R_5*SQR(cal->z0)+R_6*cal->x0*cal->y0+R_7*cal->x0*cal->z0+R_8*cal->y0*cal->z0);
	cal->b  = cal->a/sqrtf(R_4);
	cal->c  = cal->a/sqrtf(R_5);
	cal->d  = cal->a/sqrtf(R_6);
	cal->e  = cal->a/sqrtf(R_7);
	cal->f  = cal->a/sqrtf(R_8);

	return 0;
}
