# HW-Lab7 — CMSIS-DSP + CMSIS-RTOS2 (B-L475E-IOT01A2)

## What this project does

1. **Basic requirement — CMSIS-DSP FIR low-pass**
   - Uses the ARM CMSIS-DSP example coefficients (`fir1(28, 6/24)` → 6 kHz cutoff at 48 kHz sample rate).
   - At boot, runs the **known test vector** (`1 kHz + 15 kHz`) from `arm_fir_data.c` and prints **SNR** vs. the MATLAB reference (expect **> 140 dB**).
   - Then streams **LSM6DSL accelerometer X-axis** samples: CSV lines `raw,filtered` over **USART1** (ST-LINK VCP, 115200 baud).

2. **Optional Problem 1 (+10) — three CMSIS-RTOS2 tasks**
   - `SensorTask` (highest priority): periodic timer wakes task → read accelerometer.
   - `FilterTask`: CMSIS-DSP FIR on blocks of 32 samples.
   - `StreamTask`: UART CSV to PC for plotting.

## Build (STM32CubeIDE)

1. Open project `Lab7_1` (workspace must include sibling folder `STM32CubeL4`, or use the `Lab7_1/STM32CubeL4` junction).
2. Build **Debug**. If you see `FreeRTOS.h: No such file`, run **Clean** then **Build** again.
3. Flash to **B-L475E-IOT01A2**.

To disable RTOS and use a simple blocking loop, set in `Core/Inc/app_config.h`:

```c
#define USE_CMSIS_RTOS2 0
```

## Serial monitor

- Port: ST-LINK Virtual COM (e.g. `COM3` on Windows)
- Baud: **115200**
- You should see:

```text
[FIR self-test] SNR = ... dB (expect > 140)
LSM6DSL accelerometer ready
Lab7 CSV stream: raw,filtered (mg)
-0.12,0.05
...
```

## Plot on PC

```bash
pip install pyserial matplotlib
# Live rolling plot (default); close CubeIDE Serial Monitor first
python tools/plot_lab7_stream.py --port COM13
# One-shot capture of 512 points
python tools/plot_lab7_stream.py --port COM13 --snapshot --samples 512
```

## Report notes

- **Known signal test**: screenshot SNR line; optional plot of `testInput` vs `refOutput` / filter output.
- **Live sensor**: move/shake the board; plot shows high-frequency attenuation on the filtered trace.
- **RTOS**: describe three tasks, message queues, and timer-triggered sampling.
