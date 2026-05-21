#include "app_config.h"
#include "board_io.h"
#include "dsp_fir.h"
#include "lsm6dsl_min.h"
#include <stdio.h>
#include <string.h>

static volatile uint8_t sampleReady;

void Lab_TIM7_SampleCallback(void)
{
  sampleReady = 1U;
}

void Lab_RunBlocking(void)
{
  float rawBlock[DSP_FIR_BLOCK_SIZE];
  float filtBlock[DSP_FIR_BLOCK_SIZE];
  uint32_t idx = 0U;

  Board_TIM7_StartSampleTimer();
  Board_UART_Print("Lab7 blocking CSV: raw_milli,filt_milli (divide by 1000 for mg)\r\n");

  for (;;)
  {
    if (sampleReady != 0U)
    {
      sampleReady = 0U;

      if (LSM6DSL_ReadAccelX_mg(&rawBlock[idx]) == 0)
      {
        idx++;
        if (idx >= DSP_FIR_BLOCK_SIZE)
        {
          dsp_fir_process_block(rawBlock, filtBlock, DSP_FIR_BLOCK_SIZE);
          for (uint32_t i = 0; i < DSP_FIR_BLOCK_SIZE; i++)
          {
            Board_UART_PrintCsvMg(rawBlock[i], filtBlock[i]);
          }
          idx = 0U;
        }
      }
    }
  }
}
