#ifndef __LSM303__
#define __LSM303__

#include <stdint.h>
#include "vector.h"

// preca:
//   0 - 2g, 
//   1 - 4g, 
//   2 - 8g;
// precm:
//   1 - 1.3Gauss, 
//   2 - 1.9Gauss, 
//   3 - 2.5Gauss, 
//   4 - 4.0Gauss,
//   5 - 4.7Gauss,
//   6 - 5.6Gauss,
//   7 - 8.1Gauss.
int lsm303_init(int preca, int precm, int temp);

int lsm303_get_mag(pvector3d_t data);
int lsm303_get_acc(pvector3d_t data);
int lsm303_get_temp(uint16_t *temp);

#endif //__LSM303__
