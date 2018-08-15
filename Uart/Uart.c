#include "Uart.h"

UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim2;

uint8_t rx_buf[MAX_BUF_LEN] = {0};
static uint32_t rx_index = 0;

static uint32_t light;
	
void USART1_UART_Init(void)
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		assert(0);
	}
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if(huart->Instance==USART1)
	{
		/* USER CODE BEGIN USART1_MspInit 0 */

		/* USER CODE END USART1_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_USART1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**USART1 GPIO Configuration    
		PA9     ------> USART1_TX
		PA10     ------> USART1_RX 
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
		 __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
		HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
			
		
		
		__HAL_RCC_TIM2_CLK_ENABLE();
		htim2.Instance = TIM2;
		htim2.Init.Prescaler = 84000 - 1; //定时器2再APB1总线上，时钟频率84MHz,分频都时钟为1K
		htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim2.Init.Period = UART_TIME_OUT - 1;  //超时时间 200ms
		htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
		{
			assert(0);
		}
		
		__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
		
		HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM2_IRQn);
		HAL_TIM_Base_Start_IT(&htim2);
	}
}

void USART1_IRQHandler(void)
{
    uint32_t tmp_flag = 0, tmp_it_source = 0;
	tmp_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE);
	tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE);
    if((tmp_flag != RESET) && (tmp_it_source != RESET))
    {  
        rx_buf[rx_index++] = (uint8_t)(huart1.Instance->DR & (uint8_t)0x00FF);
		if(rx_index >= MAX_BUF_LEN)
			rx_index = 0;
		__HAL_TIM_SET_COUNTER(&htim2, 0);
    }
	
	
}

void TIM2_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		//处理数据 01 03 04 00 00 01 44 FB 90
		if(rx_index != 0)
		{
			light = rx_buf[5] << 8 | rx_buf[6];
			rx_index = 0;			
			//HAL_UART_Transmit(&huart1, (uint8_t*)"rece", 4, 100);
			__HAL_TIM_SET_COUNTER(&htim2, 0);
		}

	}
}
uint32_t GetLight(void)
{
	return light;
}

