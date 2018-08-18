#include <assert.h>
#include "bsp_can.h"
#include "stm32f4xx_hal.h"

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/*两个can均在APB1总线上，总线时钟42Mhz*/
/* CAN1 init function */
static void MX_CAN1_Init(void)
{
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 14;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_4TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = DISABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = DISABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan1) != HAL_OK)
        assert(0);
    
}

/* CAN2 init function */
static void MX_CAN2_Init(void)
{
    hcan2.Instance = CAN2;
    hcan2.Init.Prescaler = 14;
    hcan2.Init.Mode = CAN_MODE_NORMAL;
    hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan2.Init.TimeSeg1 = CAN_BS1_4TQ;
    hcan2.Init.TimeSeg2 = CAN_BS2_1TQ;
    hcan2.Init.TimeTriggeredMode = DISABLE;
    hcan2.Init.AutoBusOff = DISABLE;
    hcan2.Init.AutoWakeUp = DISABLE;
    hcan2.Init.AutoRetransmission = DISABLE;
    hcan2.Init.ReceiveFifoLocked = DISABLE;
    hcan2.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan2) != HAL_OK)
        assert(0);
    
}


void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if(hcan->Instance==CAN1)
    {
        /* Peripheral clock enable */
        __HAL_RCC_CAN1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**CAN1 GPIO Configuration    
        PB8     ------> CAN1_RX
        PB9     ------> CAN1_TX 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_8;//|GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_9;//|GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        __HAL_CAN_ENABLE_IT(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
        HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    }
    else if(hcan->Instance==CAN2)
    {
        /* Peripheral clock enable */
        __HAL_RCC_CAN1_CLK_ENABLE();
        __HAL_RCC_CAN2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**CAN2 GPIO Configuration    
        PB12     ------> CAN2_RX
        PB13     ------> CAN2_TX 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
           
        __HAL_CAN_ENABLE_IT(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
        HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
    }

}



static void CAN1_CANFilterConfig(void)
{
    CAN_FilterTypeDef  sFilterConfig;

    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;		//配置为掩码模式
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;	//设置为32位宽
    
    sFilterConfig.FilterIdHigh =0;
    sFilterConfig.FilterIdLow =0;
    sFilterConfig.FilterMaskIdHigh =0;
    sFilterConfig.FilterMaskIdLow =0; 		//只接收数据帧

    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;		//设置通过的数据帧进入到FIFO0中
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.FilterBank = 0;
    sFilterConfig.SlaveStartFilterBank = 14;

    if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
    {
        assert(0);
    }
}

static void CAN2_CANFilterConfig(void)
{
    CAN_FilterTypeDef  sFilterConfig;

    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;		//配置为掩码模式
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;	    //设置为32位宽
    
    sFilterConfig.FilterIdHigh     = 0 ;
    sFilterConfig.FilterIdLow      = 0 ;
    sFilterConfig.FilterMaskIdHigh = 0 ;
    sFilterConfig.FilterMaskIdLow  = 0 ; 		//只接收数据帧

    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;		//设置通过的数据帧进入到FIFO0中
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.FilterBank = 14;
    sFilterConfig.SlaveStartFilterBank = 14;

    if(HAL_CAN_ConfigFilter(&hcan2, &sFilterConfig) != HAL_OK)
    {
        assert(0);
    }
}
/*初始化两路CAN总线*/
void BSP_CAN_Init(void)
{
    MX_CAN1_Init();
    MX_CAN2_Init();
    CAN1_CANFilterConfig();
    CAN2_CANFilterConfig();
    HAL_CAN_Start(&hcan1);
    HAL_CAN_Start(&hcan2);
}

void CAN1_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan1);
}


void CAN2_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan2);
}


uint8_t buf[8] = {0};
uint32_t times = 0;

uint8_t buf2[8] = {0};
uint32_t times2 = 0;
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if(hcan->Instance == CAN1)
    {
        HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, NULL, buf);
        times++;
        printf("can1 recv = %d\n",times);
        return ;
    }
    else if(hcan->Instance == CAN2)
    {
        HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO0, NULL, buf2);
        times2++;
        printf("can2 recv = %d\n",times2);
        return ;        
    }
        
}

void CAN1_Tx(void)
{
    CAN_TxHeaderTypeDef Header;
    Header.StdId = 0x00;
    Header.ExtId = 0x00;
    Header.IDE   = CAN_ID_STD;
    Header.RTR   = CAN_RTR_DATA;   
    Header.DLC   = 8;
    Header.TransmitGlobalTime = DISABLE;
    uint8_t buf[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};   
    uint32_t txmail;
    HAL_CAN_AddTxMessage(&hcan1, &Header, buf, &txmail);
    printf("can1 tx mail = %d\n",txmail);
}

void CAN2_Tx(void)
{
    CAN_TxHeaderTypeDef Header;
    Header.StdId = 0x00;
    Header.ExtId = 0x00;
    Header.IDE   = CAN_ID_STD;
    Header.RTR   = CAN_RTR_DATA;   
    Header.DLC   = 8;
    Header.TransmitGlobalTime = DISABLE;
    uint8_t buf[8] = {0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11};   
    uint32_t txmail;
    HAL_CAN_AddTxMessage(&hcan2, &Header, buf, &txmail);
    printf("can2 tx mail = %d\n",txmail);
}