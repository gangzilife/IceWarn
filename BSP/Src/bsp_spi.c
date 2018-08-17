#include  "bsp_spi.h"

#include  "assert.h"



SPI_HandleTypeDef hspi1;



void  BSP_SPI_Init  (void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
	//__HAL_RCC_SPI1_CLK_ENABLE();
	
	
    GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_4;  //cs
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = GPIO_PIN_5; //reset
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
    
    GPIO_InitStruct.Pin = GPIO_PIN_4; //interrupt
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
	
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		assert(0);
	}
    __HAL_SPI_ENABLE(&hspi1);

}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hspi->Instance==SPI1)
  {
    __HAL_RCC_SPI1_CLK_ENABLE();
  
    /**SPI1 GPIO Configuration    
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
  }  
}


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
