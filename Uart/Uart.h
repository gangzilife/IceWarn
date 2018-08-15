#ifndef __UART_H__
#define __UART_H__

#include "stm32f4xx_hal.h"
#include "assert.h"


extern UART_HandleTypeDef huart1;

#define MAX_BUF_LEN   128
#define UART_TIME_OUT  200

void USART1_UART_Init(void);
void GetOneTimes(void);
uint32_t GetLight(void);
#endif
