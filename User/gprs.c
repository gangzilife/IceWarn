#include "MQTTClient.h"
#include "sim7600.h"
#include "mqtt_gprs_interface.h"
#include "DataConv.h"
#include "DataPub.h"
#include <assert.h>


volatile SemaphoreHandle_t xGprsMutex;
static uint8_t gprs_buf[256] = {0};
static uint8_t gprs_len = 0;
/* MQTT服务器推送的消息 */
static void messageArrived(MessageData* data)
{
//	printf("%.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
//           data->message->payloadlen, data->message->payload);
    //前六个字节 01 06 97 EC 00 00
    uint8_t* pdate = (uint8_t*)data->message->payload;
    memcpy(gprs_buf,pdate,data->message->payloadlen);
    gprs_len = data->message->payloadlen;
    for(int i = 0 ; i < gprs_len ; i++)
    {
        printf("%02X ",gprs_buf[i]);
    }
    printf("\n");
    parseNetMSG(gprs_buf,gprs_len);
}


MQTTClient gprsclient;
void vTaskCodeGPRS( void * pvParameters )
{    
    xGprsMutex = xSemaphoreCreateMutex();
    assert(xGprsMutex != NULL);
    
    (void)pvParameters;
    
    Gsm_TurnON();      //模块开机


	Network gprs_network;
    
	uint8_t sendbuf[256], readbuf[256];
	int rc = 0;

	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    gprs_NewNetwork(&gprs_network,1);
    MQTTClientInit(&gprsclient, &gprs_network, 3000,sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	char address[] = "114.55.56.64";
    //char address[] = "218.244.156.4";
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        if ((rc = gprs_ConnectNetwork(&gprs_network, (char*)address, 1883)) != 0)
        {
            printf("gprs connect fail , try again! ip = %s , port = 1883\n",address);
            continue;
        }   
        else
            break;          
    }
    printf("gprs connect success! ip = %s , port = 1883\n",address);
#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&gprsclient)) != pdPASS)
		printf("Return code from start tasks is %d\n", rc);
#endif
    
    connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "GPRS_MQTTClient";
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        if ((rc = MQTTConnect(&gprsclient, &connectData)) != 0)
        {
            printf("mqtt connect fail , try again!\n");
            continue; 
        }
        else
            break;   
    }
    printf("mqtt connect success!\n");

    char* str = "O/60568";

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        if ((rc = MQTTSubscribe(&gprsclient, (const char*)str, QOS0, messageArrived)) != 0)
        {
            printf("mqtt subcribe fail , try again! topic = %s\n",str);
            continue; 
        }
        else
            break;   
    }    
    printf("mqtt subcribe success! topic = %s\n",str);

    
	while (1)
	{
#if !defined(MQTT_TASK)
		if ((rc = MQTTYield(&gprsclient, 1000)) != 0)
			printf("Return code from yield is %d\n", rc);
#endif
        if(!gprsclient.isconnected)
        {
             //MQTT Disconnect,reconnect
//            __disable_irq();
//            HAL_NVIC_SystemReset();
            
        }
        Pub_Poll();
        vTaskDelay(pdMS_TO_TICKS(50));
	}
}