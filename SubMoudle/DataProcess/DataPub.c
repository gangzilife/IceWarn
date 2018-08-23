#include "DataPub.h"
#include "DataConv.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "MQTTClient.h"
#include "wizchip_conf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

extern SemaphoreHandle_t  xSemaphore_Pub;  
extern MQTTClient gprsclient;
extern MQTTClient client;
extern int device_id;

#define    GeneralACK           11	
#define    ONorOFF              12	
#define    OperationPasswd      13	
#define    GasSensorOK          14	
#define    GasTempature         15
#define    GasHumidity          16
#define    RoadSensorOK         17	
#define    RoadTempature        18	
#define    RoadCondition        19	
#define    PumpON               20	
#define    LiquidLevel          21	
#define    FlowLevel            22	
#define    PressureLevel        23	
#define    AutoRun              24	
#define    IceWarn              25	
#define    TimeOfIce            26	
#define    TimeOfIceWarn        27	
#define    TimeOfManual         28	
#define    SprayInterval        29	
#define    JudgInterval         30	
#define    TimeOf10g            31	
#define    SnowLimit            32	
#define    LowLiquidMask        33	
#define    LowLiquidWarn        34	
#define    LowLiquidRestore     35	
#define    HighPressMask        36	
#define    HighPressWarn        37	
#define    HighPressRestore     38	
#define    GasTempLowMask       39	
#define    GasTempLowWarn       40	
#define    GasTempLowRestore    41	
#define    RoadTempMask         42	
#define    RoadTempWarn         43	
#define    RoadTempRestore      44	
#define    RoadIceMask          45	
#define    RoadIceWarn          46	
#define    RoadIceRestore       47	
#define    TrafficErrMask       48	
#define    TrafficErrWarn       49	
#define    TrafficErrRestore    50	
#define    MagniticON           51	
#define    MagniticOFF          52	
#define    WarnEvent            53	
#define    RestoreEvent         54	


static uint8_t publish_buf[256] = { 0 };
static uint8_t publish_len = 0;

void generalACK(void)
{
	publish_buf[publish_len++] = 0x01;
	publish_buf[publish_len++] = 0x07;
	publish_buf[publish_len++] = device_id & 0xFF;
	publish_buf[publish_len++] = (device_id >> 8 ) & 0xFF;
	publish_buf[publish_len++] = (device_id >> 16) & 0xFF;
	publish_buf[publish_len++] = (device_id >> 24) & 0xFF;
	publish_buf[publish_len++] = GeneralACK;
	publish_buf[publish_len++] = 1;
    //同步发送数据
    xSemaphoreGive(xSemaphore_Pub);
}


void publish_passwd(void)
{
	publish_buf[publish_len++] = 0x01;
	publish_buf[publish_len++] = 0x07;
	publish_buf[publish_len++] = device_id & 0xFF;
	publish_buf[publish_len++] = (device_id >> 8 ) & 0xFF;
	publish_buf[publish_len++] = (device_id >> 16) & 0xFF;
	publish_buf[publish_len++] = (device_id >> 24) & 0xFF;
	publish_buf[publish_len++] = OperationPasswd;
	publish_buf[publish_len++] = SysConfig.operationpasswd & 0xFF ;
    publish_buf[publish_len++] = (SysConfig.operationpasswd >> 8 ) & 0xFF;
	publish_buf[publish_len++] = (SysConfig.operationpasswd >> 16) & 0xFF;
	publish_buf[publish_len++] = (SysConfig.operationpasswd >> 24) & 0xFF;
    //同步发送数据
    xSemaphoreGive(xSemaphore_Pub);    
}

void publish_autorun(void)
{
	publish_buf[publish_len++] = 0x01;
	publish_buf[publish_len++] = 0x07;
	publish_buf[publish_len++] = device_id & 0xFF;
	publish_buf[publish_len++] = (device_id >> 8 ) & 0xFF;
	publish_buf[publish_len++] = (device_id >> 16) & 0xFF;
	publish_buf[publish_len++] = (device_id >> 24) & 0xFF;
	publish_buf[publish_len++] = AutoRun;
	publish_buf[publish_len++] = SysConfig.autorun & 0xFF ;
    //同步发送数据
    xSemaphoreGive(xSemaphore_Pub);     
}

void publish_icewarn(void)
{
	publish_buf[publish_len++] = 0x01;
	publish_buf[publish_len++] = 0x07;
	publish_buf[publish_len++] = device_id & 0xFF;
	publish_buf[publish_len++] = (device_id >> 8 ) & 0xFF;
	publish_buf[publish_len++] = (device_id >> 16) & 0xFF;
	publish_buf[publish_len++] = (device_id >> 24) & 0xFF;
	publish_buf[publish_len++] = IceWarn;
	publish_buf[publish_len++] = SysConfig.icewarnON & 0xFF ;
    //同步发送数据
    xSemaphoreGive(xSemaphore_Pub);     
}

void publish_devicepara(void)
{
	publish_buf[publish_len++] = 0x01;
	publish_buf[publish_len++] = 0x07;
	publish_buf[publish_len++] = device_id & 0xFF;
	publish_buf[publish_len++] = (device_id >> 8 ) & 0xFF;
	publish_buf[publish_len++] = (device_id >> 16) & 0xFF;
	publish_buf[publish_len++] = (device_id >> 24) & 0xFF;
	publish_buf[publish_len++] = IceWarn;
	publish_buf[publish_len++] = SysConfig.icewarnON & 0xFF ;
    //同步发送数据
    xSemaphoreGive(xSemaphore_Pub);       
}

void Pub_Poll(void)
{
    if(xSemaphoreTake(xSemaphore_Pub,0) != pdPASS)
        return ;
    MQTTMessage message;
    message.qos = QOS0;
    message.retained = 0;
    message.payload = publish_buf;
    message.payloadlen = publish_len;
    if (MQTTPublish(&gprsclient, "I/ZJ", &message) != 0)
        printf("Publish failure\n");
    memset(publish_buf,0,sizeof(publish_buf));
    publish_len = 0;
}