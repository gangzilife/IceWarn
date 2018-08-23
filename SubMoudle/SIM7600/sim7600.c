#include "includes.h"
#include <string.h>
#include <stdlib.h>
//extern volatile RingBuffer_t Tx_buffer;
extern SemaphoreHandle_t xSemaphore_4G;

uint8_t gprs_buf[128] = {0};   //GSM接受缓冲区，全局变量

static void Gsm_RecvInit(void)
{
    memset(gprs_buf,0,sizeof(gprs_buf));
    USART2_ClearBuf();
    while(1)
    {
        if(xSemaphoreTake( xSemaphore_4G,0 ) != pdPASS)     //清除信号量
            break;
    }
}

static void Gsm_RecvCmd(void)
{
    USART2_Rx(gprs_buf,sizeof(gprs_buf));
}

//=============================================================
//函 数 名: Gsm_SendAndWait
//输入参数: cmd      :发送的命令字符串
//			strwait  :需要等待的字符串
//			num_sema :总共需要同步的信号量的个数，有的数据回应时，数据包会被分成几个部分
//			trynum   :重传的次数
//			timeout  :超时时间，信号量等待的时间
//返 回 值: 0:成功，1：失败
//功能描述: 向SIM7600 发送AT命令，并等待数据回包
//=============================================================
static uint8_t Gsm_SendAndWait(uint8_t *cmd,uint8_t *strwait,uint8_t num_sema,uint8_t trynum,uint32_t timeout)
{
    char *p;
	for(int i = 0 ; i < trynum ; i++)
	{
		//尝试发送
		Gsm_RecvInit();        //清除缓冲区
		SIM7600_SendStr(cmd);
        for(int i = 0 ; i < num_sema ; i++)
        {
            if(xSemaphoreTake( xSemaphore_4G,timeout) != pdPASS)
                break;
            Gsm_RecvCmd();
            p = strstr((char*)gprs_buf,(char*)"ERROR");
            if(p)
                break;
            else
            {
                p = strstr((char*)gprs_buf,(char*)strwait);
                if(p)
                   return 0;
            }
        }
	}
	return 1; 
}

uint8_t get_csq(uint8_t *val)
{
    char *p;
    uint8_t value = 0;
    *val = value;
    if(Gsm_SendAndWait((uint8_t *)"AT+CSQ\r\n",(uint8_t *)"+CSQ: ",1,RETRY_NUM,1000))
        return 1;
    p = strstr((char*)gprs_buf,(char*)"+CSQ:");
    if(p)
    {
        value = atoi(p+6);
    }
    *val = value;
    return 0;
}

static uint8_t Gsm_AT_CREG(uint8_t* stat)
{
	char *pst;

	if(Gsm_SendAndWait((uint8_t *)"AT+CREG?\r\n",(uint8_t *)"+CREG: ",1,RETRY_NUM,1000))
        return 1;
    pst  = strstr((char*)gprs_buf,"+CREG: "); //+CREG: 0,1
    *stat = atoi(pst+9);	
	return 0 ;
}


static uint8_t Gsm_AT_CPSI(void)
{
	char *pst , *psec;
    char *p;

    char buf[30] = {0};
	if(Gsm_SendAndWait((uint8_t *)"AT+CPSI?\r\n",(uint8_t *)"+CPSI: ",1,RETRY_NUM,2000))
        return 1;

    pst  = strstr((char*)gprs_buf,"+CPSI: ");
    psec  = strstr((char*)gprs_buf,",");
    memcpy(buf,pst,psec - pst);	
    p = strstr(buf,"NO SERVICE");
    if(p)
        return 1;
    else	
        return 0 ;
}

static uint8_t Gsm_AT_CGREG(uint8_t* stat)
{
	char *pst;

	if(Gsm_SendAndWait((uint8_t *)"AT+CGREG?\r\n",(uint8_t *)"+CGREG: ",1,RETRY_NUM,1000))
        return 1;
    pst  = strstr((char*)gprs_buf,"+CGREG: "); //+CGREG: 0,1
    *stat = atoi(pst+10);	
	return 0 ;
}

//AT+CIPRXGET=1
static uint8_t Gsm_AT_CIPRXGET(void)
{
	if(Gsm_SendAndWait((uint8_t *)"AT+CIPRXGET=1\r\n",(uint8_t *)"OK",1,RETRY_NUM,1000))
        return 1;        
	else
        return 0 ;
}


//AT+NETOPEN
static uint8_t Gsm_AT_NETOPEN(void)
{
	if(Gsm_SendAndWait((uint8_t *)"AT+NETOPEN\r\n",(uint8_t *)"+NETOPEN: 0",2,RETRY_NUM,5000))
        return 1;
    else
        return 0;
}
//AT+CIPOPEN=0,"TCP","218.244.156.4",6886
static uint8_t Gsm_AT_CIPOPEN(uint8_t *ip ,uint32_t port,uint8_t channel)
{
    char* p;
    uint8_t ret;
    uint8_t inf[50] = {0};
    sprintf((char*)inf,"AT+CIPOPEN=%d,\"TCP\",\"%s\",%d\r\n",channel,ip,port);	
	if(Gsm_SendAndWait(inf,(uint8_t *)"+CIPOPEN:",2,RETRY_NUM,5000))
        return 1;
    else
    {
        p = strstr((char*)gprs_buf,"+CIPOPEN:");
        ret = atoi(p + 12);   
        return ret;
    }
}

////AT+CGSOCKCONT=1,"IP","CMNET"
////AT+CSOCKSETPN=1
static uint8_t Gsm_Stask_Spoint(uint8_t *point)
{
	uint8_t inf[50];
	sprintf((char*)inf,"AT+CGSOCKCONT=1,\"IP\",\"%s\"\r\n",point);
	if(!Gsm_SendAndWait(inf,(uint8_t *)"OK\r\n",1,RETRY_NUM,1000))
        return  Gsm_SendAndWait((uint8_t *)"AT+CSOCKSETPN=1\r\n",(uint8_t *)"OK\r\n",1,RETRY_NUM,1000);
    else
        return 1;
}

static uint8_t Gsm_set_tcpip_app_mode(uint8_t type)
{//TCPIP应用模式(0  非透传模式    1透传模式)
	if(DEFAULT_TANS_MODE==type)
	{
		return	Gsm_SendAndWait((uint8_t *)"AT+CIPMODE=0\r\n",(uint8_t *)"OK\r\n",1,RETRY_NUM,1000);

	}
	return	Gsm_SendAndWait((uint8_t *)"AT+CIPMODE=1\r\n",(uint8_t *)"OK\r\n",1,RETRY_NUM,1000);
}

//关闭TCP/UDP连接  AT+CIPCLOSE
uint8_t Gsm_shutdowm_tcp_udp()
{
    return Gsm_SendAndWait((uint8_t *)"AT+CIPCLOSE=DEFAULT_LINK_CHANNEL\r\n",(uint8_t *)"+CIPCLOSE:\r\n",2,RETRY_NUM,5000);
}

//关闭SOCKET  AT+NETCLOSE
uint8_t Gsm_shutdowm_socket()
{
    return Gsm_SendAndWait((uint8_t *)"AT+NETCLOSE\r\n",(uint8_t *)"+NETCLOSE: 0\r\n",3,RETRY_NUM,5000);
}

///***********************************************************************************
//*连接服务器
//***********************************************************************************/
uint8_t Gsm_Connect_Server(uint8_t *ip ,uint32_t port)
{
 	uint8_t csq;
	uint8_t stat;

	if(Gsm_SendAndWait((uint8_t *)"AT\r\n",(uint8_t *)"OK\r\n",1,1,1000))
	{
		return	CONNECT_ERR_AT; 
	}
    printf("AT\n");
    if(Gsm_SendAndWait((uint8_t *)"ATE0\r\n",(uint8_t *)"OK\r\n",1,1,1000))
	{
		return	CONNECT_ERR_ATE0; 
	}
    printf("ATE0\n");
    Gsm_shutdowm_tcp_udp();
    printf("AT+CIPCLOSE=0\n");
    
    Gsm_shutdowm_socket();
    printf("AT+NETCLOSE\n");
    

 	get_csq(&csq);
    printf("AT+CSQ csq = %d\n",csq);
    if(Gsm_set_tcpip_app_mode(0))
    {
        return CONNECT_ERR_CIPMODE;
    }	
	printf("AT+CIPMODE=0\n");			
	
    if(Gsm_Stask_Spoint((uint8_t *)"CMNET"))
	{
	    return CONNECT_ERR_CSTT;
	} 
    printf("AT+CGSOCKCONT=1,IP,cmnet\n");	
    printf("AT+CSOCKSETPN=1\n");
	
	if(Gsm_AT_CREG(&stat))
	{
	    return CONNECT_ERR_CREG;	
	}
    printf("AT+CREG?\n");
 	
	if(Gsm_AT_CPSI())
	{
	    return CONNECT_ERR_CPSI;	
	}
	printf("AT+CPSI?\n");
    
	if(Gsm_AT_CGREG(&stat))
	{
	    return CONNECT_ERR_CGREG;	
	}
    printf("AT+CGREG?\n");

	if(Gsm_AT_CIPRXGET())  //手动接收字节
	{
	    return CONNECT_ERR_CIPRXGET;
	}
    printf("AT+CIPRXGET=1\n");

    if(Gsm_AT_NETOPEN())
	{
	    return CONNECT_ERR_NETOPEN;
	}		
    printf("AT+NETOPEN\n");

    if(Gsm_AT_CIPOPEN(ip,port,DEFAULT_LINK_CHANNEL))
    {
            return CONNECT_ERR_CIPOPEN;
    }
	printf("AT+CIPOPEN = %d,%s,%d\n",DEFAULT_LINK_CHANNEL,ip,port);
    
	return CONNECT_ERR_NONE;
}

uint8_t Gsm_wait(uint8_t *strwait,uint8_t num_sema,uint8_t trynum,uint32_t timeout)
{
    char *p;
    BaseType_t seam_ret = pdFAIL;
	for(int i = 0 ; i < trynum ; i++)
	{
        for(int i = 0 ; i < num_sema ; i++)
        {
            seam_ret = xSemaphoreTake( xSemaphore_4G,timeout);
            if(seam_ret != pdPASS)
                break;
            Gsm_RecvCmd();
            p = strstr((char*)gprs_buf,(char*)strwait);
            if(p)
                return 0;
        }
	}
	return 1; 
}

/**************************************************************
 *发送数据+CIPSEND: 0,5,5
 *************************************************************/
uint8_t Gsm_Send_data(uint8_t *buf, uint32_t size)
{ 
	uint8_t inf[50];

	uint8_t channel = DEFAULT_LINK_CHANNEL;

    sprintf((char*)inf,"AT+CIPSEND=%d,%d\r\n",channel,size);	


	
	if(Gsm_SendAndWait(inf,(uint8_t *)">",1,2,1000))
	{
		return CONNECT_ERR_CIPSEND;	
	}

	SIM7600_SendData(buf,size);	
	
	
	Gsm_wait((uint8_t *)"+CIPSEND: DEFAULT_LINK_CHANNEL",1,2,3000);
	
	return CONNECT_ERR_NONE;
}	




int Gsm_Recv_data(uint8_t* buf, uint16_t size)
{
    uint8_t rst;
    uint16_t len=0;
    uint16_t cnlen = 0;
    uint16_t offset = 0;
    char *pst=NULL;
    uint8_t inf[50];
    uint8_t mode = 2;
	uint8_t channel = DEFAULT_LINK_CHANNEL;
    sprintf((char*)inf,"AT+CIPRXGET=%d,%d,%d\r\n",mode,channel,size);	  
    do
    {
        rst = Gsm_SendAndWait(inf,(uint8_t *)"+CIPRXGET: 2",1,1,1000);
        if(!rst)
	    {
            pst = strstr((char*)gprs_buf,"+CIPRXGET: 2");
            pst = pst+15;
            len = atoi(pst);  //本次获取的长度
            cnlen = atoi(pst+3);	  //剩余长度
            if(len!=0)
            {            
                //pst = strstr(pst,"0x0A"); 
                pst = strchr(pst,'\n');
                memcpy(buf+offset,pst+1,len);                  
                offset += len;
                if(len == size)
                    break;
            }    
            vTaskDelay(pdMS_TO_TICKS(50));
        }
	}while((rst == 0) &&(cnlen != 0));
	return offset;
	
}

//int Gsm_Recv_data(uint8_t* buf, uint16_t size)
//{
//    char* p;
////    if(xSemaphoreTake( xSemaphore_4G,0) != pdPASS)
////        return 0;
//    //Gsm_RecvInit();
//    Gsm_RecvCmd();     //把缓冲区数据全部收入
//    p = strstr((char*)gprs_buf,(char*)"+CIPRXGET: 1");
//    if(p)
//        return Recv_data(buf, size);
//    else
//    {
//        p = strstr((char*)gprs_buf,(char*)"+IPCLOSE: "); //服务器端关闭连接，其实底层可以不用管，应用层根据心跳判断
//        if(p)
//            printf("disconnrct from server\n");
//        return 0;
//    }
//        
//}


/**************************************************************
 *模块开机接口初始化
 *************************************************************/
void Gsm_TurnON(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_RESET);

    /*Configure GPIO pin : PB1 */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    //HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(100));
    while(1)
    {
        if(Gsm_SendAndWait((uint8_t *)"AT\r\n",(uint8_t *)"OK",1,5,1000))//如果之前关机，则现在开机
        {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_RESET);
            vTaskDelay(pdMS_TO_TICKS(600));
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }
        else
        {
            break;
        }
            
    }
}


uint8_t  Gsm_CloseConnect(void)
{
    Gsm_shutdowm_tcp_udp();
    printf("AT+CIPCLOSE=0\n");
    
    Gsm_shutdowm_socket();
    printf("AT+NETCLOSE\n");    
}