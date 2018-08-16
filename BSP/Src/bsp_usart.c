#include "bsp_usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

extern SemaphoreHandle_t  xSemaphore_4G;

static UART_HandleTypeDef huart1;
static UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim2;
#define UART_TIME_OUT  100   //串口超时时间

static volatile uint16_t rx_index1 = 0;
static uint8_t rx_buf1[256] = {0};


static volatile uint16_t rx_index2 = 0;
static uint8_t rx_buf2[256] = {0};

static void USART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10    ------> USART1_RX 
    */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);

    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
    
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

static void TIM_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 8400 - 1; //定时器2再APB1总线上，时钟频率84MHz,分频都时钟为10K
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = UART_TIME_OUT * 10 - 1;  //超时时间 200ms
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
        assert(0);

    __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);

    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}
void BSP_USART_Init(void)
{
    USART_Init();  //初始化串口模块
    TIM_Init();
}

void USART1_IRQHandler(void)
{
    uint32_t tmp_flag = 0, tmp_it_source = 0;
	tmp_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE);
	tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE);
    if((tmp_flag != RESET) && (tmp_it_source != RESET))
    {
        rx_buf1[rx_index1++] = (uint8_t)(huart1.Instance->DR & (uint8_t)0x00FF);
        if(rx_index1 >= sizeof(rx_buf1))
        {
            rx_index1 = 0;
        }
    }
}


void USART2_IRQHandler(void)
{
    uint32_t tmp_flag = 0, tmp_it_source = 0;
	tmp_flag = __HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE);
	tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_RXNE);
    if((tmp_flag != RESET) && (tmp_it_source != RESET))
    {
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        if(rx_index2 == 0) //第一次接受数据，则打开定时器
            HAL_TIM_Base_Start_IT(&htim2);
        rx_buf2[rx_index2++] = (uint8_t)(huart2.Instance->DR & (uint8_t)0x00FF);
        if(rx_index2 >= sizeof(rx_buf2))
        {
            rx_index2 = 0;
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
        if(rx_index2 != 0)
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(xSemaphore_4G,&xHigherPriorityTaskWoken);
        }
        HAL_TIM_Base_Stop_IT(&htim2);
	}
}


uint16_t USART1_Rx(uint8_t* buf,uint16_t len)
{
    uint16_t ret = 0;
    if(rx_index1 <= len)
    {
        memcpy(buf,rx_buf1,rx_index1);
        ret = rx_index1;
        rx_index1 = 0;       
    }
    else
    {
        memcpy(buf,rx_buf1,len);
        ret = len;
        rx_index1 -= len;
        memcpy(rx_buf1,rx_buf1 + len,rx_index1);
    }
    return ret;
}


uint16_t USART2_Rx(uint8_t* buf,uint16_t len)
{
    uint16_t ret = 0;
    if(rx_index2 <= len)
    {
        memcpy(buf,rx_buf2,rx_index2);
        ret = rx_index2;
        rx_index2 = 0;       
    }
    else
    {
        memcpy(buf,rx_buf2,len);
        ret = len;
        rx_index2 -= len;
        memcpy(rx_buf2,rx_buf2 + len,rx_index2);
    }
    return ret;
}

void USART2_ClearBuf(void)
{
    memset(rx_buf2,0,sizeof(rx_buf2));
    rx_index2 = 0;
}

void USART1_Tx(uint8_t* buf,uint16_t size)
{
    HAL_UART_Transmit(&huart1, buf, size, (size + 9)/10);
}

void USART2_Tx(uint8_t* buf,uint16_t size)
{
    HAL_UART_Transmit(&huart2, buf, size, (size + 9)/10);
}
