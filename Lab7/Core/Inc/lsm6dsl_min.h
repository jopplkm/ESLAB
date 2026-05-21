#ifndef LSM6DSL_MIN_H
#define LSM6DSL_MIN_H

#include <stdint.h>

int LSM6DSL_Init(void);
int LSM6DSL_ReadAccelX_mg(float *ax_mg);

#endif
