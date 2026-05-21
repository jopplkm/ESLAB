#include "dsp_fir.h"
#include "arm_fir_data.h"
#include "math_helper.h"

static const float32_t firCoeffs32[DSP_FIR_NUM_TAPS] = {
  -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f, +0.0085302217f, -0.0000000000f, -0.0173976984f,
  -0.0341458607f, -0.0333591565f, +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f, +0.2229246956f,
  +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f, -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f,
  +0.0080754303f, +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f
};

static arm_fir_instance_f32 firInstance;
static float32_t firState[DSP_FIR_BLOCK_SIZE + DSP_FIR_NUM_TAPS - 1];

void dsp_fir_init(void)
{
  arm_fir_init_f32(&firInstance, DSP_FIR_NUM_TAPS, (float32_t *)firCoeffs32, firState, DSP_FIR_BLOCK_SIZE);
}

void dsp_fir_process_block(const float32_t *input, float32_t *output, uint32_t blockSize)
{
  arm_fir_f32(&firInstance, (float32_t *)input, output, blockSize);
}

float dsp_fir_run_self_test(void)
{
  static float32_t testOutput[320];
  uint32_t i;
  float snr;

  dsp_fir_init();

  for (i = 0; i < (320U / DSP_FIR_BLOCK_SIZE); i++)
  {
    dsp_fir_process_block(&testInput_f32_1kHz_15kHz[i * DSP_FIR_BLOCK_SIZE],
                          &testOutput[i * DSP_FIR_BLOCK_SIZE],
                          DSP_FIR_BLOCK_SIZE);
  }

  snr = arm_snr_f32(refOutput, testOutput, 320U);
  return snr;
}
