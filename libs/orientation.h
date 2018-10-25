#ifndef __ORIENTATION_H__
#define __ORIENTATION_H__

#include "vector.h"

int calc_angles(pvector3df_t acc, pvector3df_t mag, float *tilt, float *azimuth);

#endif //__ORIENTATION_H__
