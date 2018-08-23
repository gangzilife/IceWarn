#include "includes.h"

/*4G 模块无线任务，main函数无条件创建*/
extern void vTaskCodeGPRS( void * pvParameters );  

/*以太网任务，只有在DHCP成功时才会创建*/
extern void vTaskCodeETH( void * pvParameters );     

/*气象任务，处理气象站数据和配置信息*/
extern void vTaskCodeWeater( void * pvParameters );     
/*主线程，负责创建其他线程*/
void vTaskCodeMain( void * pvParameters );

/*UART2(4G串口)异步通知信号量*/
SemaphoreHandle_t  xSemaphore_4G;  
SemaphoreHandle_t  xSemaphore_weater;

/*publish同步信号量*/
SemaphoreHandle_t  xSemaphore_Pub;  

int main(void)
{
	HAL_Init();
	BSP_SystemClkCfg();
    BSP_Init();
    Debug_Init();
    EEPROM_Init();
    //Pumb_Init();
    
	TaskHandle_t xHandleMain;
    BaseType_t err = xTaskCreate( vTaskCodeMain,"Main",130,NULL,2,&xHandleMain );
    assert(err == pdPASS);

	TaskHandle_t xHandleGprs;
    err = xTaskCreate( vTaskCodeGPRS,"gprs",512,NULL,2,&xHandleGprs );
    assert(err == pdPASS);

	TaskHandle_t xHandleWeater;
    err = xTaskCreate( vTaskCodeWeater,"Weater",256,NULL,2,&xHandleWeater );
    assert(err == pdPASS);

    TaskHandle_t xEnternetHandle;
    err = xTaskCreate( vTaskCodeETH,"EnternetMQTT",512,NULL,3,&xEnternetHandle);
    assert(err == pdPASS);
    
    xSemaphore_4G = xSemaphoreCreateCounting( 20, 0 );
    assert(xSemaphore_4G != NULL);
    
    xSemaphore_weater = xSemaphoreCreateBinary();
    assert(xSemaphore_weater != NULL);

    xSemaphore_Pub = xSemaphoreCreateBinary();
    assert(xSemaphore_Pub != NULL);
    
    vTaskStartScheduler();
	return 0;
}


void vTaskCodeMain( void * pvParameters )
{
    (void)pvParameters;

//    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
//    vTaskDelay(pdMS_TO_TICKS(200));
//    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
//    vTaskDelay(pdMS_TO_TICKS(1000));

    
    while(1)
    {   
        vTaskDelay(pdMS_TO_TICKS(20));
               
    }
}


void HardFault_Handler(void)
{
  while(1)
  {
  }
}



