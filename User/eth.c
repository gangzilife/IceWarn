#include <string.h>
#include "MQTTClient.h"
#include "wizchip_conf.h"
#include "dhcp.h"
#include "DataConv.h"
#include "DataPub.h"
#define DHCP_SOCKET   0                   //DHCP  socket,DHCP完成后会close


static uint8_t eth_buf[256] = {0};
static uint8_t eth_len = 0;
static void messageArrived(MessageData* data)
{
//	printf("%.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
//           data->message->payloadlen, data->message->payload);
    //前六个字节 01 06 97 EC 00 00
    uint8_t* pdate = (uint8_t*)data->message->payload;
    memcpy(eth_buf,pdate,data->message->payloadlen);
    eth_len = data->message->payloadlen;
    for(int i = 0 ; i < eth_len ; i++)
    {
        printf("%02X ",eth_buf[i]);
    }
    printf("\n");
    parseNetMSG(eth_buf,eth_len);
}

/*DHCP所需要的缓冲区和IP缓存*/
uint8_t ip[4] = {0};
uint8_t dhcp_buf[1024] = {0};
MQTTClient client;
void vTaskCodeETH( void * pvParameters )
{
    
    wizchip_init(NULL,NULL);
    DHCP_init(DHCP_SOCKET,dhcp_buf);    
    /*底层网线连接机制，如果网线没连接则不进行后续操作*/
    while(1)
    {
        if(wizphy_getphylink() == PHY_LINK_ON)
            break;      
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    while(1)
    {
        if(DHCP_run() == DHCP_IP_LEASED)
        {
            DHCP_stop();
            getIPfromDHCP(ip);
            printf("DHCP ip addr = %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
            
	Network network;
    
	uint8_t sendbuf[256], readbuf[256];
	int rc = 0;
    
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    NewNetwork(&network,1);
    MQTTClientInit(&client, &network, 3000,sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	//uint8_t address[] = {192,168,0,142};   
    //char address[] = "114.55.56.64";
    uint8_t address[] = {114,55,56,64};   
    while(1)
    {
        if ((rc = ConnectNetwork(&network, (char*)address, 1883)) != 1)
            printf("Return code from network connect is %d , try again 1s later!\n", rc); 
        else
        {
            printf("ConnectNetwork Success. ServerIP = %d.%d.%d.%d,port = %d\n", address[0],address[1],address[2],address[3],1883); 
            break;
        } 
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    
#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&client)) != pdPASS)
		printf("Return code from start tasks is %d\n", rc);
#endif
    connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "Enternet_MQTTClient";

    while(1)
    {
        if ((rc = MQTTConnect(&client, &connectData)) != 0)
            printf("Return code from MQTT connect is %d , tryagain!\n", rc);
        else
        {
            printf("MQTT Connected\n");     
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }


    char* sub = "O/60568";
    while(1)
    {
        if ((rc = MQTTSubscribe(&client, sub, QOS0, messageArrived)) != 0)
            printf("Return code from MQTT subscribe is %d , try again \n", rc); 
        else
        {
            printf("MQTT Subscribe success,topic = %s\n",sub);     
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }    
	while (1)
	{
        
//		MQTTMessage message;
//		char payload[30];
//
//		message.qos = QOS1;
//		message.retained = 0;
//		message.payload = payload;
//		sprintf(payload, "%d", count++);
//		message.payloadlen = strlen(payload);
//
//		if ((rc = MQTTPublish(&client, "sensor", &message)) != 0)
//			printf("Return code from MQTT publish is %d\n", rc);
#if !defined(MQTT_TASK)
		if ((rc = MQTTYield(&client, 1000)) != 0)
			printf("Return code from yield is %d\n", rc);
#endif
        if(!client.isconnected)
        {
             printf("MQTT Disconnect,reconnecting!!!\n");//MQTT Disconnect,reconnect
        }
        Pub_Poll();
        vTaskDelay(pdMS_TO_TICKS(50));
	}
}