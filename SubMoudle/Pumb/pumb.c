#include <stdint.h>
#include <assert.h>
#include "stm32f4xx_hal.h"


UART_HandleTypeDef huart5;
TIM_HandleTypeDef htim2;

#define UART_TIME_OUT  200   //串口超时时间


/*驱动层缓冲区及缓存索引*/
static uint8_t rx_buf[256] = {0};
static uint16_t rx_index = 0;

void Pumb_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_UART5_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct;
    /**UART5 GPIO Configuration    
    PC12     ------> UART5_TX
    PD2     ------> UART5_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    
    huart5.Instance = UART5;
    huart5.Init.BaudRate = 9600;
    huart5.Init.WordLength = UART_WORDLENGTH_8B;
    huart5.Init.StopBits = UART_STOPBITS_1;
    huart5.Init.Parity = UART_PARITY_NONE;
    huart5.Init.Mode = UART_MODE_TX_RX;
    huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart5.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart5) != HAL_OK)
        assert(0);
    

    
    __HAL_UART_ENABLE_IT(&huart5, UART_IT_RXNE);
    HAL_NVIC_SetPriority(UART5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(UART5_IRQn);
    /**************************************************************************/
    /*     一下初始化一个定时器，用做串口数据分包                             */
    /**************************************************************************/
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 8400 - 1; //定时器2再APB1总线上，时钟频率84MHz,分频都时钟为10K
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = UART_TIME_OUT * 10 - 1;  //超时时间 200ms
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
        assert(0);

    __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);

    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void UART5_IRQHandler(void)
{
    uint32_t tmp_flag = 0, tmp_it_source = 0;
	tmp_flag = __HAL_UART_GET_FLAG(&huart5, UART_FLAG_RXNE);
	tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart5, UART_IT_RXNE);
    if((tmp_flag != RESET) && (tmp_it_source != RESET))
    {
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        if(rx_index == 0) //第一次接受数据，则打开定时器
            HAL_TIM_Base_Start_IT(&htim2);
        rx_buf[rx_index++] = (uint8_t)(huart5.Instance->DR & (uint8_t)0x00FF);
		if(rx_index >= sizeof(rx_buf)) //缓冲区溢出,把之前收到的数据全部抛弃，重新获取
        {
            rx_index = 0;
            HAL_TIM_Base_Stop_IT(&htim2);
        }
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
        if(rx_index != 0)
        {
            for(int i = 0 ; i < rx_index ; i++)
                printf("%02X ",rx_buf[i]);
            printf("\n");
            rx_index = 0;
        }
        HAL_TIM_Base_Stop_IT(&htim2);
	}
    else if(htim->Instance == TIM5)
	{
		if(rx_index_485 != 0) //驱动层收到了数据，将数据拷贝至应用层处理
		{
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            /*计算传感器数据*/
            //xSemaphoreGive(xSemaphore_roadCondition);
            xSemaphoreGiveFromISR(xSemaphore_roadCondition,&xHigherPriorityTaskWoken);
			HAL_TIM_Base_Stop_IT(&htim2);
		}
	}
}

void Pumb_Tx(void)
{
    uint8_t tx_buf[10] = {1,2,3,4,5,6,7,8,9,10};
    HAL_UART_Transmit(&huart5, tx_buf, sizeof(tx_buf), (sizeof(tx_buf) + 9 ) / 10);
}