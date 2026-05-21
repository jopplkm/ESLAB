#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* Set to 1 for optional Problem 1 (three CMSIS-RTOS2 tasks). */
#ifndef USE_CMSIS_RTOS2
#define USE_CMSIS_RTOS2 1
#endif

/* 1 = run CMSIS-DSP FIR self-test on known 1 kHz + 15 kHz signal at boot. */
#define LAB_RUN_FIR_SELF_TEST 1

/* Sample rate for accelerometer stream (Hz). */
#define LAB_SAMPLE_RATE_HZ 100U

#endif
