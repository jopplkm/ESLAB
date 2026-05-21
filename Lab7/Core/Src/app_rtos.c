#include "app_rtos.h"
#include "app_config.h"
#include "board_io.h"
#include "dsp_fir.h"
#include "lsm6dsl_min.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <string.h>

typedef struct
{
  float raw;
  float filtered;
} dsp_stream_sample_t;

#define RAW_QUEUE_DEPTH   8U
#define STREAM_QUEUE_DEPTH 8U

static osMessageQueueId_t rawSampleQueue;
static osMessageQueueId_t streamQueue;
static osThreadId_t sensorTaskHandle;
static osThreadId_t filterTaskHandle;
static osThreadId_t streamTaskHandle;
static osTimerId_t sampleTimer;

static float filterIn[DSP_FIR_BLOCK_SIZE];
static float filterOut[DSP_FIR_BLOCK_SIZE];
static uint32_t filterFill;

static void SensorTask(void *argument);
static void FilterTask(void *argument);
static void StreamTask(void *argument);
static void SampleTimerCallback(void *argument);

void App_RTOS_Start(void)
{
  const osMessageQueueAttr_t rawQueueAttr = {.name = "rawQ"};
  const osMessageQueueAttr_t streamQueueAttr = {.name = "streamQ"};

  rawSampleQueue = osMessageQueueNew(RAW_QUEUE_DEPTH, sizeof(float), &rawQueueAttr);
  streamQueue = osMessageQueueNew(STREAM_QUEUE_DEPTH, sizeof(dsp_stream_sample_t), &streamQueueAttr);

  dsp_fir_init();

  const osThreadAttr_t sensorAttr = {
    .name = "SensorTask",
    .priority = osPriorityRealtime,
    .stack_size = 4 * 512
  };
  const osThreadAttr_t filterAttr = {
    .name = "FilterTask",
    .priority = osPriorityAboveNormal,
    .stack_size = 4 * 1024
  };
  const osThreadAttr_t streamAttr = {
    .name = "StreamTask",
    .priority = osPriorityNormal,
    .stack_size = 4 * 512
  };

  osKernelInitialize();

  sensorTaskHandle = osThreadNew(SensorTask, NULL, &sensorAttr);
  filterTaskHandle = osThreadNew(FilterTask, NULL, &filterAttr);
  streamTaskHandle = osThreadNew(StreamTask, NULL, &streamAttr);

  sampleTimer = osTimerNew(SampleTimerCallback, osTimerPeriodic, NULL, NULL);
  osTimerStart(sampleTimer, 1000U / LAB_SAMPLE_RATE_HZ);

  osKernelStart();
}

static void SampleTimerCallback(void *argument)
{
  (void)argument;
  osThreadFlagsSet(sensorTaskHandle, 0x1U);
}

static void SensorTask(void *argument)
{
  float sample;

  (void)argument;

  for (;;)
  {
    (void)osThreadFlagsWait(0x1U, osFlagsWaitAny, osWaitForever);

    if (LSM6DSL_ReadAccelX_mg(&sample) == 0)
    {
      (void)osMessageQueuePut(rawSampleQueue, &sample, 0U, 0U);
    }
  }
}

static void FilterTask(void *argument)
{
  float sample;
  dsp_stream_sample_t out;

  (void)argument;
  filterFill = 0U;

  for (;;)
  {
    if (osMessageQueueGet(rawSampleQueue, &sample, NULL, osWaitForever) == osOK)
    {
      filterIn[filterFill++] = sample;

      if (filterFill >= DSP_FIR_BLOCK_SIZE)
      {
        dsp_fir_process_block(filterIn, filterOut, DSP_FIR_BLOCK_SIZE);

        {
          uint32_t i;
          for (i = 0; i < DSP_FIR_BLOCK_SIZE; i++)
          {
            out.raw = filterIn[i];
            out.filtered = filterOut[i];
            (void)osMessageQueuePut(streamQueue, &out, 0U, osWaitForever);
          }
        }

        filterFill = 0U;
      }
    }
  }
}

static void StreamTask(void *argument)
{
  dsp_stream_sample_t sample;

  (void)argument;

  Board_UART_Print("Lab7 CSV stream: raw_milli,filt_milli (divide by 1000 for mg)\r\n");

  for (;;)
  {
    if (osMessageQueueGet(streamQueue, &sample, NULL, osWaitForever) == osOK)
    {
      Board_UART_PrintCsvMg(sample.raw, sample.filtered);
    }
  }
}
