#ifndef DSP_FIR_H
#define DSP_FIR_H

#include "arm_math.h"
#include <stdint.h>

#define DSP_FIR_NUM_TAPS   29U
#define DSP_FIR_BLOCK_SIZE   32U

void dsp_fir_init(void);
void dsp_fir_process_block(const float32_t *input, float32_t *output, uint32_t blockSize);
float dsp_fir_run_self_test(void);

#endif
