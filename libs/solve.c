#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <math.h>

#include "solve.h"

float matrix_source[6*MAX_K];
float matrix_x2[MAX_K];
float matrix_trans[6*MAX_K];

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

static int inverse(float *data)
{
	int gaus_ostatok;
	float gaus_deter,gaus_minved;

	//   Based on realization:
	//  (c) Drobotenko    http://drobotenko.com

	int rn[6],cn[6];
	int j,k;

	for (j=6;j--;)
		rn[j]=cn[j]=j;

#if defined( CORTEX )
	gaus_minved=FLT_MAX/10;
#else
	gaus_minved=1e99;
#endif
	gaus_deter=1;

	for(gaus_ostatok=6;gaus_ostatok;gaus_ostatok--)
	{
		int jved=0,kved=0;
		int jt,kt;
		float vved=-1;
		float t;

		// поиск ведущего
		for(j=6;j--;)
			if(~rn[j])
				for(k=6;k--;)
					if(~cn[k])
						if(vved<fabsf(data[j+6*k]))
						{
							vved = fabsf(data[j+6*k]);
							jved = j;
							kved = k;
						}

		if(gaus_minved>vved)
			gaus_minved=vved;

		gaus_deter*=data[jved+6*kved];

#if defined( CORTEX )
		if(vved < (FLT_MIN*10))
#else
		if(vved < 1e-99)
#endif
		{
			for(j=6;j--;)
			{
				if(~rn[j])
					for(k=6;k--;)
						data[j+6*k]=NAN;
				if(~cn[j])
					for(k=6;k--;)
						data[k+6*j]=NAN;
			}
			return 1;
		}

		jt=rn[jved];
		kt=cn[kved];

		// перестановки
		for(j=6;j--;)
		{
			t=data[kt+6*j];
			data[kt+6*j]=data[jved+6*j];
			data[jved+6*j]=t;
		}

		for(j=6;j--;)
		{
			t=data[j+6*jt];
			data[j+6*jt]=data[j+6*kved];
			data[j+6*kved]=t;
		}

		rn[jved]=rn[kt];
		cn[kved]=cn[jt];
		rn[kt]=cn[jt]=-1;
		vved=data[kt+6*jt];
		data[kt+6*jt]=1;

		for(j=6;j--;)
		{
			float mul;
			if(j==kt)
				continue;
			mul=data[j+6*jt]/vved;
			data[j+6*jt]=0;
			for(k=6;k--;)
				data[j+6*k]-=data[kt+6*k]*mul;
		}

		for(k=6;k--;)
			data[kt+6*k]/=vved;
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

int solve_fill_int(pvector3d_t values, int k)
{
	int i;
	
	for (i = 0; i < k; i++)
	{
		matrix_source[i*6+0] = (float)values[i].x;
		matrix_source[i*6+1] = (float)values[i].y;
		matrix_source[i*6+2] = (float)values[i].z;
		matrix_source[i*6+3] = (float)-SQR(values[i].y);
		matrix_source[i*6+4] = (float)-SQR(values[i].z);
		matrix_source[i*6+5] = 1;
		matrix_x2[i]         = (float)SQR(values[i].x);
	}
	return 0;
}

int solve_fill_float(pvector3df_t values, int k)
{
	int i;
	
	for (i = 0; i < k; i++)
	{
		matrix_source[i*6+0] = (float)values[i].x;
		matrix_source[i*6+1] = (float)values[i].y;
		matrix_source[i*6+2] = (float)values[i].z;
		matrix_source[i*6+3] = (float)-SQR(values[i].y);
		matrix_source[i*6+4] = (float)-SQR(values[i].z);
		matrix_source[i*6+5] = 1;
		matrix_x2[i]         = (float)SQR(values[i].x);
	}
	return 0;
}

int solve_linear_system(pcalibrate_t cal, int k)
{
	int ret;
	float res[6];
	float matrix_forinv[6*6];

	// Решаем систему:

	// 1. Ищем псевообратное: A+ = (A* A)^-1 A*:
	ret = transpose(matrix_source, 6, k, matrix_trans);
	if (ret) return ret;
	multiply(matrix_trans, k, 6, matrix_source, 6, k, matrix_forinv);
	inverse(matrix_forinv);

	// 2. Непосредственно решаем систему:
	multiply(matrix_forinv, 6, 6, matrix_trans, k, 6, matrix_source);
	multiply(matrix_source, k, 6, matrix_x2, 1, k, res);
	
	// Сохраняем коэффициенты:
	cal->offset.x = res[0] / 2;
	cal->offset.y = res[1] / 2 / res[3];
	cal->offset.z = res[2] / 2 / res[4];
	cal->koeff.x  = sqrtf(res[5] + SQR(cal->offset.x) + SQR(cal->offset.y)*res[3] + SQR(cal->offset.z)*res[4]);
	cal->koeff.y  = cal->koeff.x / sqrt(res[3]);
	cal->koeff.z  = cal->koeff.x / sqrt(res[4]);

	return 0;
}
