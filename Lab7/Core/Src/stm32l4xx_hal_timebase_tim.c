#include "app_config.h"

#if USE_CMSIS_RTOS2

#include "stm32l4xx_hal.h"

TIM_HandleTypeDef TimHandle;

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  RCC_ClkInitTypeDef clkconfig;
  uint32_t uwTimclock;
  uint32_t uwAPB1Prescaler;
  uint32_t uwPrescalerValue;
  uint32_t pFLatency;
  HAL_StatusTypeDef status = HAL_OK;

  __HAL_RCC_TIM6_CLK_ENABLE();

  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
  uwAPB1Prescaler = clkconfig.APB1CLKDivider;
  uwTimclock = (uwAPB1Prescaler == RCC_HCLK_DIV1) ? HAL_RCC_GetPCLK1Freq() : (2U * HAL_RCC_GetPCLK1Freq());
  uwPrescalerValue = (uint32_t)((uwTimclock / 1000000U) - 1U);

  TimHandle.Instance = TIM6;
  TimHandle.Init.Period = (1000000U / 1000U) - 1U;
  TimHandle.Init.Prescaler = uwPrescalerValue;
  TimHandle.Init.ClockDivision = 0;
  TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  HAL_TIM_Base_MspInit(&TimHandle);
  if (HAL_TIM_Base_Init(&TimHandle) == HAL_OK)
  {
    if (HAL_TIM_Base_Start_IT(&TimHandle) == HAL_OK)
    {
      HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
      if (TickPriority < (1UL << __NVIC_PRIO_BITS))
      {
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, TickPriority, 0U);
        uwTickPrio = TickPriority;
      }
      else
      {
        status = HAL_ERROR;
      }
    }
    else
    {
      status = HAL_ERROR;
    }
  }
  else
  {
    status = HAL_ERROR;
  }

  return status;
}

void HAL_SuspendTick(void)
{
  __HAL_TIM_DISABLE_IT(&TimHandle, TIM_IT_UPDATE);
}

void HAL_ResumeTick(void)
{
  __HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
}

void TIM6_DAC_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimHandle);
}

#endif /* USE_CMSIS_RTOS2 */
