#include "includes.h"
#include <string.h>
#include <stdlib.h>
//extern volatile RingBuffer_t Tx_buffer;
extern volatile SemaphoreHandle_t xGprsMutex;

uint8_t gprs_buf[128] = {0};   //GSM接受缓冲区，全局变量

static void Gsm_RecvInit(void)
{
    for(uint8_t i = 0 ; i < sizeof(gprs_buf) ; i++)
    {
        gprs_buf[i] = 0;
    }
}

static void Gsm_RecvCmd(void)
{
    USART2_Rx(gprs_buf,sizeof(gprs_buf));
}

uint8_t Gsm_SendAndWait(uint8_t *cmd,uint8_t *strwait,uint8_t trynum,uint8_t timeout)
{
    uint8_t   i ;
    
	char *p;
    
    const portTickType xDelay = (50*timeout) / portTICK_RATE_MS;
	xSemaphoreTake( xGprsMutex,portMAX_DELAY );

	for(i=0;i<trynum;i++)
	{
		//尝试发送
		Gsm_RecvInit();
		SIM7600_SendStr(cmd);
		vTaskDelay(xDelay);
		//首先处理从收的数据
		Gsm_RecvCmd();
		p = strstr((char*)gprs_buf,(char*)strwait);
		if(p)
        {
		   xSemaphoreGive(xGprsMutex);
		   return 0;
        }
	}
	xSemaphoreGive(xGprsMutex);
	return 1; 
}

uint8_t Gsm_SendAndWait2(uint8_t *cmd,uint8_t *strwait,uint8_t* strwait2,uint8_t trynum,uint8_t timeout)
{
    uint8_t   i ;
    
	char *p;
    
    const portTickType xDelay = (50*timeout) / portTICK_RATE_MS;
	xSemaphoreTake( xGprsMutex,portMAX_DELAY );

	for(i=0;i<trynum;i++)
	{
		//尝试发送
		Gsm_RecvInit();
		SIM7600_SendStr(cmd);
		vTaskDelay(xDelay);
		//首先处理从收的数据
		Gsm_RecvCmd();
		p = strstr((char*)gprs_buf,(char*)strwait);
		if(p)
        {
		   xSemaphoreGive(xGprsMutex);
		   return 0;
        }
        else
        {
            p = strstr((char*)gprs_buf,(char*)strwait2);
            if(p)
            {
               xSemaphoreGive(xGprsMutex);
               return 0;
            }
        }
	}
	xSemaphoreGive(xGprsMutex);
	return 1; 
}

uint8_t Gsm_wait(uint8_t *strwait,uint8_t trynum,uint8_t timeout)
{
    uint8_t   i;
	char *p;	
    
    const portTickType xDelay = (50*timeout) / portTICK_RATE_MS;
	xSemaphoreTake( xGprsMutex,portMAX_DELAY );
    
	for(i=0;i<trynum;i++)
	{
		vTaskDelay(xDelay);
        Gsm_RecvCmd();
		p = strstr((char*)gprs_buf,(char*)strwait);
		if(p)
		{
		   xSemaphoreGive(xGprsMutex);
		   return 0;
		}
	}
	xSemaphoreGive(xGprsMutex);
	return 1; 
}

uint8_t Gsm_set(uint8_t *cmd)
{
	uint8_t result;
	result = Gsm_SendAndWait(cmd,(uint8_t *)"OK\r\n",RETRY_NUM,5);
	return result;
}


void Gsm_csq(uint8_t* csq)
{
	uint8_t   rst;
	char *pst;
	uint8_t   temp;

	rst =Gsm_SendAndWait((uint8_t *)"AT+CSQ\r\n",(uint8_t *)"+CSQ: ",RETRY_NUM,1);
	if(!rst)
	{
		pst  = strstr((char*)gprs_buf,"+CSQ:");
		temp = atoi(pst+6);	
		*csq = temp;
	}
}

//关闭TCP/UDP连接  AT+CIPCLOSE
uint8_t Gsm_shutdowm_tcp_udp()
{
    if(GPRS_CIPMUX_TYPE) //多连接
        return Gsm_set((uint8_t *)"AT+CIPCLOSE=DEFAULT_LINK_CHANNEL\r\n");
    else
        return Gsm_set((uint8_t *)"AT+CIPCLOSE\r\n");
}

//关闭移动场景	AT+CIPSHUT	
static uint8_t Gsm_close_moving_scene()
{
    return  Gsm_SendAndWait((uint8_t *)"AT+CIPSHUT\r\n",(uint8_t *)"SHUT OK\r\n",RETRY_NUM,20);
}


static uint8_t Gsm_set_tcpip_app_mode(uint8_t type)
{//TCPIP应用模式(0  非透传模式    1透传模式)
	if(DEFAULT_TANS_MODE==type)
	{
		return	Gsm_SendAndWait((uint8_t *)"AT+CIPMODE=0\r\n",(uint8_t *)"OK\r\n",RETRY_NUM,1);

	}
	return	Gsm_SendAndWait((uint8_t *)"AT+CIPMODE=1\r\n",(uint8_t *)"OK\r\n",RETRY_NUM,1);
}


//启动任务并设置移动接入点。AT+CSTT="CMNET"
static uint8_t Gsm_Stask_Spoint(uint8_t *point)
{
	uint8_t inf[30];
	sprintf((char*)inf,"AT+CSTT=\"%s\"\r\n",point);
	return	Gsm_SendAndWait(inf,(uint8_t *)"OK\r\n",RETRY_NUM,10);

}


static uint8_t Gsm_active_moving_scene()
{//激活移动场景。AT+CIICR	
	return Gsm_SendAndWait((uint8_t *)"AT+CIICR\r\n",(uint8_t *)"OK\r\n",RETRY_NUM,100);
}

uint8_t Gsm_get_local_ip()
{//获取本地的IP.	AT+CIFSR
	return	Gsm_SendAndWait((uint8_t *)"AT+CIFSR\r\n",(uint8_t *)"OK\r\n",RETRY_NUM,1);
}

/*
uint8_t Gsm_set_local_port(uint32_t port)
{//设置本地端口号	AT+CLPORT="TCP","2200"
	uint8_t inf[50];
	sprintf((char*)inf,"AT+CLPORT=\"TCP\",\"%d\"\r\n",port);	
	
	return Gsm_set(inf);;
}*/

static uint8_t Gsm_Connect_Tcp_or_UdpPort(uint8_t *ip ,uint32_t port,uint8_t channel)
{
	uint8_t inf[50];
	
	if(channel==DEFAULT_LINK_CHANNEL)   //多连接
	{
		sprintf((char*)inf,"AT+CIPSTART=%d,\"TCP\",\"%s\",\"%d\"\r\n",channel,ip,port);		
	}	
	else
	{
		sprintf((char*)inf,"AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r\n",ip,port);
	}	
		
	return Gsm_SendAndWait2(inf,(uint8_t *)"CONNECT OK",(uint8_t *)"ALREADY CONNECT" ,RETRY_NUM,40);	
}


static uint8_t Gsm_Set_SendData_BackInf(uint8_t TYPE)
{
	if(DEFAULT_SENDDATA_BACKINF_MODE==TYPE)	
	{
		return Gsm_set((uint8_t *)"AT+CIPSPRT=1\r\n");
	}
	return Gsm_set((uint8_t *)"AT+CIPSPRT=2\r\n"); //发送成功时不显示'<',不返回"send ok"
}

static uint8_t Gsm_AT_CREG(uint8_t* stat)
{
	uint8_t   rst;
	char *pst;

	rst =Gsm_SendAndWait((uint8_t *)"AT+CREG?\r\n",(uint8_t *)"+CREG: ",RETRY_NUM,1);
	if(!rst)
	{
		pst  = strstr((char*)gprs_buf,"+CREG: "); //+CREG: 0,1
		*stat = atoi(pst+9);	
	}
	return rst ;
}

static uint8_t Gsm_AT_CSMINS(uint8_t* stat)
{
	uint8_t   rst;
	char *pst;

	rst =Gsm_SendAndWait((uint8_t *)"AT+CSMINS?\r\n",(uint8_t *)"+CSMINS: ",RETRY_NUM,1);
	if(!rst)
	{
		pst  = strstr((char*)gprs_buf,"+CSMINS: "); //+CSMINS: : 1,1 //0-未插入   1-插入
		*stat = atoi(pst+11);	
	}
	return rst ;
}

static uint8_t Gsm_AT_CGATT(uint8_t* stat)
{
	uint8_t   rst;
	char *pst;

	Gsm_SendAndWait((uint8_t *)"AT+CGATT=1\r\n",(uint8_t *)"OK",RETRY_NUM,40);

	
	rst =Gsm_SendAndWait((uint8_t *)"AT+CGATT?\r\n",(uint8_t *)"+CGATT: ",RETRY_NUM,1);
	if(!rst)
	{
		pst  = strstr((char*)gprs_buf,"+CGATT: "); //+CGATT: 1 //0-分离   1-附着
		*stat = atoi(pst+8);	
	}
	return rst ;
}



/**************************************************************
 *初始化模块
 *************************************************************/
uint8_t Gsm_Init(void)
{  
	uint8_t   stat;
    
	//初始化缓冲区
    Gsm_RecvInit();
	if(Gsm_SendAndWait((uint8_t *)"AT\r\n",(uint8_t *)"OK",1,10))
	{
		return CONNECT_ERR_AT;
	}
    
	//printf("设置无回显 \r\n");
	if(Gsm_set((uint8_t *)"ATE0\r\n"))
	{
		return CONNECT_ERR_ATE0;
	}

 	//printf("设置波特率 \r\n");
	//if(!Gsm_set((uint8_t *)"AT+IPR=115200\r\n"))
	//{
	    //return CONNECT_ERR_IPR;
	//}
	

	//printf("开启SIM卡检测 \r\n");
	if(Gsm_set((uint8_t *)"AT+CSDT=1\r\n"))
	{
	    return CONNECT_ERR_CSDT;
	}
    
	//printf("SIM卡插入状态报告: \r\n");
	Gsm_AT_CSMINS(&stat);
	//printf("%d:\r\n",stat);
	if(stat!=0x01) //0未插入  1插入
	{
		return CONNECT_ERR_CSMINS;	
	}
    
	return CONNECT_ERR_NONE;
}

/**************************************************************
 *模块开机接口初始化
 *************************************************************/
void Gsm_TurnON(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

    /*Configure GPIO pin : PB1 */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(100));
    while(1)
    {
        if(Gsm_SendAndWait((uint8_t *)"AT\r\n",(uint8_t *)"OK",1,10))//如果之前关机，则现在开机
        {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_RESET);
            vTaskDelay(pdMS_TO_TICKS(1000));
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        else
            break;
    }
}

/***********************************************************************************
*连接服务器
***********************************************************************************/
uint8_t Gsm_Connect_Server(uint8_t *ip ,uint32_t port)
{
 	uint8_t csq;
	uint8_t stat;
	uint8_t mux = GPRS_CIPMUX_TYPE;

	if(Gsm_SendAndWait((uint8_t *)"AT\r\n",(uint8_t *)"OK\r\n",1,5))
	{
		return	CONNECT_ERR_AT; 
	}

 	//printf("关闭TCP/UDP连接 \r\n");	
    Gsm_shutdowm_tcp_udp();


 	//printf("关闭移动场景 \r\n");
    Gsm_close_moving_scene();

	if(Gsm_set((uint8_t *)"AT+CIPRXGET=1\r\n"))  //手动接收字节
	{
	    return CONNECT_ERR_CIPRXGET;
	}    

 	Gsm_csq(&csq);
 	//printf("GSM SIGNAL QUALITY REPORT : %d \r\n",csq);
	//AT+CSQ



	//printf("NETWORK REGISTRATION \r\n");
	if(Gsm_AT_CREG(&stat))
	{
	    return CONNECT_ERR_CREG;	
	}
	//printf("REPORT: %d\r\n",stat);			
	//AT+CREG?
 	
	if(Gsm_AT_CGATT(&stat))
	{
	    return CONNECT_ERR_CGATT;	
	}
	
 	//printf("启动多连接   0单连接   1 多连接 \r\n");
	if(1==mux)
	{
		if(Gsm_set((uint8_t *)"AT+CIPMUX=1\r\n"))
		{
			return CONNECT_ERR_CIPMUX;
		}	
	}
	else
	{
		if(Gsm_set((uint8_t *)"AT+CIPMUX=0\r\n"))
		{
			return CONNECT_ERR_CIPMUX;
		}	
		//printf("TCPIP应用模式(0  非透明模式    1透明模式 \r\n");
		if(Gsm_set_tcpip_app_mode(0))
		{
				return CONNECT_ERR_CIPMODE;
		}	
	}			
	

 	//printf("启动任务并设置移动接入点 \r\n");
    if(Gsm_Stask_Spoint((uint8_t *)"CMNET"))
	{
	    return CONNECT_ERR_CSTT;
	}


 	//printf("激活移动场景 \r\n");
    if(Gsm_active_moving_scene())
	{
	    return CONNECT_ERR_CIICR;
	}		

 	//printf("获取本地的IP \r\n");
    if(Gsm_get_local_ip())
	{
	    //return CONNECT_ERR_CIFSR;
	}
	
	
	//printf("设置jt808服务端IP地址和端口号 \r\n");			
	if(1==mux)
	{
		if(Gsm_Connect_Tcp_or_UdpPort(ip,port,DEFAULT_LINK_CHANNEL))
		{
				return CONNECT_ERR_CIPSTART;
		}
	}
	else
	{
		if(Gsm_Connect_Tcp_or_UdpPort(ip,port,0xff))
		{
				return CONNECT_ERR_CIPSTART;
		}
	}	
	//printf("设置数据发送回显示 \r\n");
	if(Gsm_Set_SendData_BackInf(1))
	{
	    return CONNECT_ERR_CIPSPRT;
	}		
	
	return CONNECT_ERR_NONE;
}

/**************************************************************
 *发送数据
 *************************************************************/
uint8_t Gsm_Send_data(uint8_t *s, uint32_t size)
{ 
	uint8_t inf[50];
	uint8_t mux = GPRS_CIPMUX_TYPE;
	uint8_t channel = DEFAULT_LINK_CHANNEL;
	if(1==mux)
	{
		sprintf((char*)inf,"AT+CIPSEND=%d,%d\r\n",channel,size);	
	}
	else
	{
		sprintf((char*)inf,"AT+CIPSEND=%d\r\n",size);	
	}	

	
	if(Gsm_SendAndWait(inf,(uint8_t *)">",RETRY_NUM,1))
	{
		return CONNECT_ERR_CIPSEND;	
	}

	SIM7600_SendData(s,size);	
	
	
	Gsm_wait((uint8_t *)"SEND OK",1,4);
	
	return CONNECT_ERR_NONE;
}	

uint16_t Gsm_Recv_data(uint8_t* buf, uint16_t size)
{
    //OS_ERR      err;
    uint8_t rst;
    uint16_t len=0;
    uint16_t cnlen = 0;
    uint16_t offset = 0;
    char *pst=NULL;
    uint8_t inf[50];
    uint8_t mode = 2;
	uint8_t mux = GPRS_CIPMUX_TYPE;
	uint8_t channel = DEFAULT_LINK_CHANNEL;
	if(1==mux)
		sprintf((char*)inf,"AT+CIPRXGET=%d,%d,%d\r\n",mode,channel,size);	
	else
		sprintf((char*)inf,"AT+CIPRXGET=%d,%d\r\n",mode,size);	   
    do
    {
        rst = Gsm_SendAndWait(inf,(uint8_t *)"+CIPRXGET: 2",RETRY_NUM,1);
        if(!rst)
	    {
            pst = strstr((char*)gprs_buf,"+CIPRXGET: 2");
            pst = pst+15;
            len = atoi(pst);
            cnlen = atoi(pst+3);	        
            if(len!=0)
            {            
                //pst = strstr(pst,"0x0A"); 
                pst = strchr(pst,'\n');
                memcpy(buf+offset,pst+1,len);                  
                offset += len;
            }    
            vTaskDelay(pdMS_TO_TICKS(50));
        }
	}while((cnlen != 0)&&(rst == 1));
	return offset;
	
}
uint8_t Gsm_CloseConnect()
{
    return Gsm_shutdowm_tcp_udp();
}