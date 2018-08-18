#include "includes.h"

void vTaskCodeWeater( void * pvParameters )
{
    (void)pvParameters;
    /*初始化结冰预警模块*/
    InitIcewarnModule();
    /*添加用于计算的60个虚拟数据*/
    float data = 30;
    for(int i = 0 ; i < 30; i++)
    {
         AddDataToModule(data);
         data += 0.5;
    }
    for(int i = 0 ; i < 30; i++)
    {
         AddDataToModule(data);
         data -= 0.5;
    }
    uint32_t tick = 0;
    float dew_point = 0;
    float u = 0;
    
    while(1)
    {
        if(xTaskGetTickCount() - tick >= 1*60*1000)
        {
            dew_point = Get_Dewpoint(-14,70);
            printf("dew_point = %.4f ",dew_point);
            u = get_u();
            printf("u = %.4f\n",u);  
            tick = xTaskGetTickCount();
        }        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}