#include "board_io.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c2;
UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim7;

void Board_Peripherals_Init(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_I2C2_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_TIM7_CLK_ENABLE();

  /* I2C2: PB10 SCL, PB11 SDA (on-board sensors) */
  gpio.Pin = INTERNAL_I2C2_SCL_Pin | INTERNAL_I2C2_SDA_Pin;
  gpio.Mode = GPIO_MODE_AF_OD;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOB, &gpio);

  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00702681; /* B-L475E-IOT01 BSP I2C2 timing @ 80 MHz */
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  HAL_I2C_Init(&hi2c2);

  /* USART1: ST-LINK VCP PB6 TX, PB7 RX */
  gpio.Pin = ST_LINK_UART1_TX_Pin | ST_LINK_UART1_RX_Pin;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOB, &gpio);

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart1);

  /* TIM7: 100 Hz sample tick (80 MHz timer clock -> PSC for 100 kHz, ARR for 100 Hz) */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 799;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 999;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim7);
}

void Board_TIM7_StartSampleTimer(void)
{
  HAL_NVIC_SetPriority(TIM7_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(TIM7_IRQn);
  HAL_TIM_Base_Start_IT(&htim7);
}

void Board_UART_Print(const char *msg)
{
  const uint8_t *p = (const uint8_t *)msg;
  uint16_t len = (uint16_t)strlen(msg);
  /* HAL_MAX_DELAY: avoid dependency on HAL tick when TIM6 not ready yet */
  (void)HAL_UART_Transmit(&huart1, (uint8_t *)p, len, HAL_MAX_DELAY);
}

void Board_UART_PrintCsvMg(float raw_mg, float filtered_mg)
{
  char line[32];
  int raw_milli = (int)(raw_mg * 1000.0f);
  int filt_milli = (int)(filtered_mg * 1000.0f);
  /* %d only: newlib nano often omits %f (shows empty -> ",," lines) */
  snprintf(line, sizeof(line), "%d,%d\r\n", raw_milli, filt_milli);
  Board_UART_Print(line);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C2)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C2_CLK_ENABLE();
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    __HAL_RCC_TIM6_CLK_ENABLE();
  }
  else if (htim->Instance == TIM7)
  {
    __HAL_RCC_TIM7_CLK_ENABLE();
  }
}
