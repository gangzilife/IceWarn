#include "includes.h"


SemaphoreHandle_t  xSemaphore_4G;

int cnt = 0;
void vTaskCode( void * pvParameters )
{
    (void)pvParameters;

//    /*初始化结冰预警模块*/
//    InitIcewarnModule();
//    /*添加用于计算的60个虚拟数据*/
//    float data = 30;
//    for(int i = 0 ; i < 30; i++)
//    {
//         AddDataToModule(data);
//         data += 0.5;
//    }
//    for(int i = 0 ; i < 30; i++)
//    {
//         AddDataToModule(data);
//         data -= 0.5;
//    }
//    uint32_t tick = 0;
//    float ret = 0;
//    float u = 0;
    
    uint8_t tbuf[18] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};
    uint8_t rbuf[256] = {0};
    //EEPROM_Write(0, tbuf, sizeof(tbuf));
    //EEPROM_Read(0, rbuf, sizeof(rbuf));
    BSP_LED_On(0);
    Gsm_TurnON();
    cnt = Gsm_Connect_Server("218.244.156.4" ,6886);  //218.244.156.4",6886
    if(!cnt)
    {
        Gsm_Send_data(tbuf, sizeof(tbuf));
    }
    int recv_len = 0;
    while(1)
    {        
        //BSP_LED_Toggle(0);
//        tick = xTaskGetTickCount();
//        ret = Get_Dewpoint(-14,70);
//        printf("tick = %d ,%.4f\n",xTaskGetTickCount() - tick,ret);
//        tick = xTaskGetTickCount();
//        u = get_u();
//        printf("tick = %d ,%.4f\n",xTaskGetTickCount() - tick,u);
//        Pumb_Tx();
        recv_len = Gsm_Recv_data(rbuf, sizeof(rbuf));
        if(recv_len)
        {
            for(int i = 0 ; i < recv_len ; i++)
            {
                printf("%02X ",rbuf[i]);
            }
            printf("\n");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main(void)
{
	HAL_Init();
	BSP_SystemClkCfg();
    BSP_Init();
    Debug_Init();
    EEPROM_Init();
    //Pumb_Init();
    
	TaskHandle_t xHandle;
    BaseType_t err = xTaskCreate( vTaskCode,"demo",130,NULL,2,&xHandle );
    assert(err == pdPASS);
    
    xSemaphore_4G = xSemaphoreCreateCounting( 10, 0 );
    assert(xSemaphore_4G != NULL);
    vTaskStartScheduler();
	return 0;
}


void HardFault_Handler(void)
{
  while(1)
  {
  }
}



