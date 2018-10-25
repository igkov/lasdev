#ifndef __SOLVE9_H__
#define __SOLVE9_H__

#include "vector.h"

#define MAX_K 32

int solve_fill_int9(pvector3d_t values, int k);
int solve_fill_float9(pvector3df_t values, int k);
int solve_linear_system9(pcalibrate9_t cal, int k);

#endif // __SOLVE9_H__
