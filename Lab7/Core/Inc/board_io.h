#ifndef BOARD_IO_H
#define BOARD_IO_H

#include "main.h"

extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim7;

void Board_Peripherals_Init(void);
void Board_TIM7_StartSampleTimer(void);
void Board_UART_Print(const char *msg);
/* CSV line in milli-mg integers (avoids float printf with nano.specs). */
void Board_UART_PrintCsvMg(float raw_mg, float filtered_mg);

#endif
