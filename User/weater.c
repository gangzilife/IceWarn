#include "includes.h"

extern SemaphoreHandle_t  xSemaphore_weater;

float road_tempature;
uint8_t road_condition;
float gas_humi;
float gas_temp;


__packed typedef struct
{
    uint16_t head;
    uint16_t len;
    uint16_t frameNum;
    uint8_t  roaderr;
    uint8_t  roadtemp[4];
    uint8_t  roadcon;
    uint8_t  gaserr;
    uint8_t  temp[4];
    uint8_t  humi[4];
    uint16_t crc;
}DataFrame_t;


static unsigned short crc_modbus(unsigned char *arr_buff, int  len)
{
	unsigned short crc = 0xFFFF;
	for (int j = 0; j < len;j++)
	{
		crc = crc ^ arr_buff[j];
		for (int i = 0; i<8; i++)
		{
			if ((crc & 0x0001) >0)
			{
				crc = crc >> 1;
				crc = crc ^ 0xa001;
			}
			else
				crc = crc >> 1;
		}
	}
	return (crc);
}

/*解析气象数据，包括温湿度，路面温度和路面状态*/
void parseWeaterMSG(uint8_t* buf,uint8_t len , uint8_t *gas,uint8_t *road)
{
    if(len == 0)
        return ;
    DataFrame_t* pbuf = (DataFrame_t*)buf;
    uint16_t crc = crc_modbus(buf,len - 2);
    if(crc != pbuf->crc)
    {
        *gas = 1;
        *road = 1;
        return ;        
    }
    *gas = pbuf->gaserr;
    if(pbuf->gaserr == 0)
    {
        memcpy((uint8_t*)&gas_temp,pbuf->temp,sizeof(gas_temp));              
        memcpy((uint8_t*)&gas_humi,pbuf->humi,sizeof(gas_humi));
        printf("Tempature is %.4f,humidity is %.2f%%\n",gas_temp,gas_humi);  
    }
    else
        printf("tempuature and humidity sensor data err!\n");
    *road = pbuf->roaderr;
    if(pbuf->roaderr == 0)
    {
        memcpy((uint8_t*)&road_tempature,pbuf->roadtemp,sizeof(road_tempature));    
        road_condition = pbuf->roadcon;
        printf("Road Tempature is %.4f,road condition is %d\n",road_tempature,road_condition);     
    }
    else
        printf("road condition sensor data err!\n");
}


void vTaskCodeWeater( void * pvParameters )
{
    (void)pvParameters;
    /*初始化结冰预警模块*/
    InitIcewarnModule();
    InitRoadCondModule();
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
    
    float dew_point = 0;
    float u = 0;
    
    uint8_t  weater_buf[64] = {0};
    uint16_t weater_len = 0;
    
    uint8_t gas_err  = 1;
    uint8_t road_err = 1;
    while(1)
    {
        if(xSemaphoreTake( xSemaphore_weater,0 ) == pdPASS)
        {
            weater_len = USART1_Rx(weater_buf,sizeof(weater_buf));
            parseWeaterMSG(weater_buf,weater_len , &gas_err,&road_err);
            if(gas_err == 0)
            {
                dew_point = Get_Dewpoint(gas_temp,gas_humi);
                printf("dew_point = %.4f\n ",dew_point);                
            }
            if(road_err == 0)
            {
                AddDataToModule(road_tempature);
                u = get_u();
                printf("u = %.4f\n",u);  
                
                AddDataToRoadCond((float)road_condition);
                if(RoadCond_Ice())
                {
                    printf("have ice\n");
                }
                    
            }
        } 
        
        /*一分钟向平台上报一次当前气象站信息，包括六项*/
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}