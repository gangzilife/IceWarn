#include "includes.h"
#include "eth.h"
#include "gprs.h"


SemaphoreHandle_t  xSemaphore_4G;
#define DHCP_SOCKET   0                   //DHCP  socket


int cnt = 0;
uint8_t ip[4] = {0};
uint8_t dhcp_buf[1024] = {0};
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
    
//    uint8_t tbuf[18] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};
//    uint8_t rbuf[256] = {0};
    //EEPROM_Write(0, tbuf, sizeof(tbuf));
    //EEPROM_Read(0, rbuf, sizeof(rbuf));
//    BSP_LED_On(0);
    
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(100));
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(1000));
    wizchip_init(NULL,NULL);
    DHCP_init(DHCP_SOCKET,dhcp_buf);
    
//    Gsm_TurnON();
//    cnt = Gsm_Connect_Server("218.244.156.4" ,6886);  //218.244.156.4",6886
//    if(!cnt)
//    {
//        printf("connect ok\n");
//        Gsm_Send_data(tbuf, sizeof(tbuf));
//    }
//    int recv_len = 0;
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
//        recv_len = Gsm_Recv_data(rbuf, sizeof(rbuf));
//        if(recv_len > 0)
//        {
//            for(int i = 0 ; i < recv_len ; i++)
//            {
//                printf("%02X ",rbuf[i]);
//            }
//            printf("\n");
//        }
//        else if(recv_len < 0)
//            printf("disconnect\n");
        
        
        if(DHCP_run() == DHCP_IP_LEASED)
        {
            DHCP_stop();
            getIPfromDHCP(ip);
            TaskHandle_t xEnternetHandle;
            BaseType_t err = xTaskCreate( vTaskCodeETH,"EnternetMQTT",256,NULL,3,&xEnternetHandle);
            assert(err == pdPASS);
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
    
	TaskHandle_t xHandleDemo;
    BaseType_t err = xTaskCreate( vTaskCode,"demo",130,NULL,2,&xHandleDemo );
    assert(err == pdPASS);

	TaskHandle_t xHandleGprs;
    err = xTaskCreate( vTaskCodeGPRS,"gprs",256,NULL,2,&xHandleGprs );
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



