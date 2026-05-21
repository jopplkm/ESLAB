#include "lsm6dsl_min.h"
#include "board_io.h"

#define LSM6DSL_ADDR        (0xD4U)
#define LSM6DSL_WHO_AM_I    0x0FU
#define LSM6DSL_WHO_AM_I_VAL 0x6AU
#define LSM6DSL_CTRL1_XL    0x10U
#define LSM6DSL_CTRL3_C     0x12U
#define LSM6DSL_OUTX_L_XL   0x28U

static uint8_t lsm6dsl_read_reg(uint8_t reg)
{
  uint8_t value = 0;
  HAL_I2C_Mem_Read(&hi2c2, LSM6DSL_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
  return value;
}

static void lsm6dsl_write_reg(uint8_t reg, uint8_t value)
{
  HAL_I2C_Mem_Write(&hi2c2, LSM6DSL_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
}

int LSM6DSL_Init(void)
{
  if (lsm6dsl_read_reg(LSM6DSL_WHO_AM_I) != LSM6DSL_WHO_AM_I_VAL)
  {
    return -1;
  }

  /* 52 Hz ODR, +/-2 g, block update, auto-increment */
  lsm6dsl_write_reg(LSM6DSL_CTRL1_XL, 0x40U);
  lsm6dsl_write_reg(LSM6DSL_CTRL3_C, 0x44U);
  return 0;
}

int LSM6DSL_ReadAccelX_mg(float *ax_mg)
{
  uint8_t raw[2];
  int16_t axis;

  if (HAL_I2C_Mem_Read(&hi2c2, LSM6DSL_ADDR, LSM6DSL_OUTX_L_XL, I2C_MEMADD_SIZE_8BIT, raw, 2, 100) != HAL_OK)
  {
    return -1;
  }

  axis = (int16_t)((uint16_t)raw[1] << 8 | raw[0]);
  *ax_mg = ((float)axis * 0.061f);
  return 0;
}
