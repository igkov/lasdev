#ifndef __SOLVE_H__
#define __SOLVE_H__

#include "vector.h"

#define MAX_K 32

int solve_fill_int(pvector3d_t values, int k);
int solve_fill_float(pvector3df_t values, int k);
int solve_linear_system(pcalibrate_t cal, int k);

#endif // __SOLVE_H__
