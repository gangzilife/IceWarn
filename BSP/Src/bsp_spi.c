#include  "bsp_spi.h"
#include  "stm32f4xx_hal.h"
#include  "os.h"
#include  "assert.h"

#define USE_DMA   0 

static SPI_HandleTypeDef hspi1;

#if USE_DMA == 1
static DMA_HandleTypeDef hdma_tx;
static DMA_HandleTypeDef hdma_rx;
static OS_SEM sem;
#endif

void  BSP_SPI_Init  (void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_SPI1_CLK_ENABLE();
	
#if USE_DMA == 1
	__HAL_RCC_DMA1_CLK_ENABLE();
#endif
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_15, GPIO_PIN_SET);
    
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		assert(0);
	}
#if USE_DMA == 1	
	hdma_tx.Instance                 = DMA1_Channel3;
	hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma_tx.Init.Mode                = DMA_NORMAL;
	hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;

	HAL_DMA_Init(&hdma_tx);

	/* Associate the initialized DMA handle to the the SPI handle */
	__HAL_LINKDMA(&hspi1, hdmatx, hdma_tx);

	hdma_rx.Instance                 = DMA1_Channel2;

	hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma_rx.Init.Mode                = DMA_NORMAL;
	hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;

	HAL_DMA_Init(&hdma_rx);

	/* Associate the initialized DMA handle to the the SPI handle */
	__HAL_LINKDMA(&hspi1, hdmarx, hdma_rx);
	
	HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 3);
	HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

	HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 3);
	HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
	
	OS_ERR err;
	OSSemCreate(&sem, "SPI1 Sem", 1, &err);
	assert(err == OS_ERR_NONE);
#endif
}




/*
void  BSP_SPI_Trans(uint8_t *buf, uint16_t len)
{
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, buf, buf, len, (len + 9)/ 10);
	assert(status == HAL_OK);
}
*/
#if USE_DMA == 1	

void BSP_SPI_TxRx(const uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
{
	OS_ERR err;
	OSSemPend(&sem, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
	assert(err == OS_ERR_NONE);
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t*)pTxData, pRxData, Size);
	assert(status == HAL_OK);
	
}

void BSP_SPI_Tx(const uint8_t *pData, uint16_t Size)
{
	OS_ERR err;
	OSSemPend(&sem, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
	assert(err == OS_ERR_NONE);
	HAL_StatusTypeDef status = HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*)pData, Size);
	assert(status == HAL_OK);
}

void BSP_SPI_Rx(uint8_t *pData, uint16_t Size)
{
	OS_ERR err;
	OSSemPend(&sem, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
	assert(err == OS_ERR_NONE);
	HAL_StatusTypeDef status = HAL_SPI_Receive_DMA(&hspi1, pData, Size);
	assert(status == HAL_OK);	
}
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	OS_ERR err;
	OSSemPost(&sem, OS_OPT_POST_1, &err);
}
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	OS_ERR err;
	OSSemPost(&sem, OS_OPT_POST_1, &err);
}
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	OS_ERR err;
	OSSemPost(&sem, OS_OPT_POST_1, &err);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	assert(0);
}

void DMA1_Channel3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hspi1.hdmatx);
}

void DMA1_Channel2_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hspi1.hdmarx);
}
#else
void BSP_SPI_TxRx(const uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
{
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)pTxData, pRxData, Size, (Size + 9)/ 10);
	assert(status == HAL_OK);	
}

void BSP_SPI_Tx(const uint8_t *pData, uint16_t Size)
{
	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, (uint8_t*)pData, Size, (Size + 9)/ 10);
	assert(status == HAL_OK);
}

void BSP_SPI_Rx(uint8_t *pData, uint16_t Size)
{
	HAL_StatusTypeDef status = HAL_SPI_Receive(&hspi1, pData, Size, (Size + 9)/ 10);
	assert(status == HAL_OK);	
}
#endif


