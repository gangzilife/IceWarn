#ifndef  __BSP_USART_H_
#define  __BSP_USART_H_

#include <assert.h>
#include "stm32f4xx_hal.h"
#include "string.h"



void BSP_USART_Init(void);


void USART2_Tx(uint8_t* buf,uint16_t size);
uint16_t USART2_Rx(uint8_t* buf,uint16_t len);
void USART2_ClearBuf(void);

void USART1_Tx(uint8_t* buf,uint16_t size);
uint16_t USART1_Rx(uint8_t* buf,uint16_t len);

#endif  