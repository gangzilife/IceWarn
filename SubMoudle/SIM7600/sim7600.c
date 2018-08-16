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
uint8_t Gsm_SendAndWait(uint8_t *cmd,uint8_t *strwait,uint8_t num_sema,uint8_t trynum,uint32_t timeout)
{
    char *p;
    BaseType_t seam_ret = pdFAIL;
	for(int i = 0 ; i < trynum ; i++)
	{
		//尝试发送
		Gsm_RecvInit();        //清除缓冲区
		while(1)
		{
			if(xSemaphoreTake( xSemaphore_4G,0 ) != pdPASS)     //清除信号量
				break;
		}
		SIM7600_SendStr(cmd);
        for(int i = 0 ; i < num_sema ; i++)
        {
            seam_ret = xSemaphoreTake( xSemaphore_4G,timeout);
            if(seam_ret != pdPASS)
                return 1;
        }
		Gsm_RecvCmd();
		p = strstr((char*)gprs_buf,(char*)strwait);
		if(p)
		   return 0;
	}
	return 1; 
}

//uint8_t Gsm_SendAndWait2(uint8_t *cmd,uint8_t *strwait,uint8_t* strwait2,uint8_t trynum,uint8_t timeout)
//{
//    uint8_t   i ;
//    
//	char *p;
//    
//    const portTickType xDelay = (50*timeout) / portTICK_RATE_MS;
//	xSemaphoreTake( xSemaphore_4G,portMAX_DELAY );
//
//	for(i=0;i<trynum;i++)
//	{
//		//尝试发送
//		Gsm_RecvInit();
//		SIM7600_SendStr(cmd);
//		vTaskDelay(xDelay);
//		//首先处理从收的数据
//		Gsm_RecvCmd();
//		p = strstr((char*)gprs_buf,(char*)strwait);
//		if(p)
//        {
//		   xSemaphoreGive(xSemaphore_4G);
//		   return 0;
//        }
//        else
//        {
//            p = strstr((char*)gprs_buf,(char*)strwait2);
//            if(p)
//            {
//               xSemaphoreGive(xSemaphore_4G);
//               return 0;
//            }
//        }
//	}
//	xSemaphoreGive(xSemaphore_4G);
//	return 1; 
//}
//
//uint8_t Gsm_wait(uint8_t *strwait,uint8_t trynum,uint8_t timeout)
//{
//    uint8_t   i;
//	char *p;	
//    
//    const portTickType xDelay = (50*timeout) / portTICK_RATE_MS;
//	xSemaphoreTake( xSemaphore_4G,portMAX_DELAY );
//    
//	for(i=0;i<trynum;i++)
//	{
//		vTaskDelay(xDelay);
//        Gsm_RecvCmd();
//		p = strstr((char*)gprs_buf,(char*)strwait);
//		if(p)
//		{
//		   xSemaphoreGive(xSemaphore_4G);
//		   return 0;
//		}
//	}
//	xSemaphoreGive(xSemaphore_4G);
//	return 1; 
//}
//
//uint8_t Gsm_set(uint8_t *cmd)
//{
//	uint8_t result;
//	result = Gsm_SendAndWait(cmd,(uint8_t *)"OK\r\n",RETRY_NUM,5);
//	return result;
//}
//
//
//void Gsm_csq(uint8_t* csq)
//{
//	uint8_t   rst;
//	char *pst;
//	uint8_t   temp;
//
//	rst =Gsm_SendAndWait((uint8_t *)"AT+CSQ\r\n",(uint8_t *)"+CSQ: ",RETRY_NUM,1);
//	if(!rst)
//	{
//		pst  = strstr((char*)gprs_buf,"+CSQ:");
//		temp = atoi(pst+6);	
//		*csq = temp;
//	}
//}
//
////关闭TCP/UDP连接  AT+CIPCLOSE
//uint8_t Gsm_shutdowm_tcp_udp()
//{
//    return Gsm_set((uint8_t *)"AT+CIPCLOSE=DEFAULT_LINK_CHANNEL\r\n");
//}
//
////关闭SOCKET  AT+NETCLOSE
//uint8_t Gsm_shutdowm_socket()
//{
//    return Gsm_set((uint8_t *)"AT+NETCLOSE\r\n");
//}
//
//
//
//
//static uint8_t Gsm_set_tcpip_app_mode(uint8_t type)
//{//TCPIP应用模式(0  非透传模式    1透传模式)
//	if(DEFAULT_TANS_MODE==type)
//	{
//		return	Gsm_SendAndWait((uint8_t *)"AT+CIPMODE=0\r\n",(uint8_t *)"OK\r\n",RETRY_NUM,1);
//
//	}
//	return	Gsm_SendAndWait((uint8_t *)"AT+CIPMODE=1\r\n",(uint8_t *)"OK\r\n",RETRY_NUM,1);
//}
//
//
////AT+CGSOCKCONT=1,"IP","CMNET"
////AT+CSOCKSETPN=1
//static uint8_t Gsm_Stask_Spoint(uint8_t *point)
//{
//	uint8_t inf[50];
//	sprintf((char*)inf,"AT+CGSOCKCONT=1,\"IP\",\"%s\"\r\n",point);
//	if(!Gsm_SendAndWait(inf,(uint8_t *)"OK\r\n",RETRY_NUM,10))
//        return  Gsm_SendAndWait((uint8_t *)"AT+CSOCKSETPN=1\r\n",(uint8_t *)"OK\r\n",RETRY_NUM,1);
//    else
//        return 1;
//
//}
//
//
//
//static uint8_t Gsm_Connect_Tcp_or_UdpPort(uint8_t *ip ,uint32_t port,uint8_t channel)
//{
//	uint8_t inf[50];
//
//    
//	
//    if(Gsm_SendAndWait2((uint8_t *)"AT+NETOPEN\r\n",(uint8_t *)"OK",(uint8_t *)"Network is already opened" ,RETRY_NUM,4))
//        return 1;
//    
//    sprintf((char*)inf,"AT+CIPOPEN=%d,\"TCP\",\"%s\",\"%d\"\r\n",channel,ip,port);		
//		
//	return Gsm_SendAndWait(inf,(uint8_t *)"OK",RETRY_NUM,20);	
//}
//
//
//
//static uint8_t Gsm_AT_CREG(uint8_t* stat)
//{
//	uint8_t   rst;
//	char *pst;
//
//	rst =Gsm_SendAndWait((uint8_t *)"AT+CREG?\r\n",(uint8_t *)"+CREG: ",RETRY_NUM,1);
//	if(!rst)
//	{
//		pst  = strstr((char*)gprs_buf,"+CREG: "); //+CREG: 0,1
//		*stat = atoi(pst+9);	
//	}
//	return rst ;
//}
//
//
//
//static uint8_t Gsm_AT_CPSI(uint8_t* stat)
//{
//	uint8_t   rst = 1;
//	char *pst , *psec;
//    char *p;
//
//    char buf[30] = {0};
//	rst = Gsm_SendAndWait((uint8_t *)"AT+CPSI?\r\n",(uint8_t *)"OK",RETRY_NUM,2);
//
//	if(!rst)
//	{
//		pst  = strstr((char*)gprs_buf,"+CPSI: ");
//        psec  = strstr((char*)gprs_buf,",");
//		memcpy(buf,pst,psec - pst);	
//        p = strstr(buf,"NO SERVICE")
//        if(p)
//            *stat = 1;
//        else
//            *stat = 0;
//	}
//	return rst ;
//}
//
//
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
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(100));
    while(1)
    {
        if(Gsm_SendAndWait((uint8_t *)"AT\r\n",(uint8_t *)"OK",1,1,1000))//如果之前关机，则现在开机
        {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_RESET);
            vTaskDelay(pdMS_TO_TICKS(500));
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        else
        {
            Gsm_SendAndWait((uint8_t *)"ATE0\r\n",(uint8_t *)"OK",1,1,1000);
            break;
        }
            
    }
}
//
///***********************************************************************************
//*连接服务器
//***********************************************************************************/
//uint8_t Gsm_Connect_Server(uint8_t *ip ,uint32_t port)
//{
// 	uint8_t csq;
//	uint8_t stat;
//
//	if(Gsm_SendAndWait((uint8_t *)"AT\r\n",(uint8_t *)"OK\r\n",1,5))
//	{
//		return	CONNECT_ERR_AT; 
//	}
//
//    if(Gsm_SendAndWait((uint8_t *)"ATE0\r\n",(uint8_t *)"OK\r\n",1,5))
//	{
//		return	CONNECT_ERR_ATE0; 
//	}
//    
// 	//printf("关闭TCP/UDP连接 \r\n");	
//    Gsm_shutdowm_tcp_udp();
//
//
//	if(Gsm_set((uint8_t *)"AT+CIPRXGET=1\r\n"))  //手动接收字节
//	{
//	    return CONNECT_ERR_CIPRXGET;
//	}    
//
// 	Gsm_csq(&csq);
// 	//printf("GSM SIGNAL QUALITY REPORT : %d \r\n",csq);
//	//AT+CSQ
//
//
//
//	//printf("NETWORK REGISTRATION \r\n");
//	if(Gsm_AT_CREG(&stat))
//	{
//	    return CONNECT_ERR_CREG;	
//	}
//	//printf("REPORT: %d\r\n",stat);			
//	//AT+CREG?
// 	
//	if(Gsm_AT_CPSI(&stat))
//	{
//	    return CONNECT_ERR_CPSI;	
//	}
//	
//
//    //printf("TCPIP应用模式(0  非透明模式    1透明模式 \r\n");
//    if(Gsm_set_tcpip_app_mode(0))
//    {
//            return CONNECT_ERR_CIPMODE;
//    }	
//				
//	
//
// 	//printf("启动任务并设置移动接入点 \r\n");
//    if(Gsm_Stask_Spoint((uint8_t *)"CMNET"))
//	{
//	    return CONNECT_ERR_CSTT;
//	}
//
//
// 	//printf("激活移动场景 \r\n");
//    if(Gsm_active_moving_scene())
//	{
//	    return CONNECT_ERR_CIICR;
//	}		
//
// 	//printf("获取本地的IP \r\n");
//    if(Gsm_get_local_ip())
//	{
//	    //return CONNECT_ERR_CIFSR;
//	}
//	
//	
//
//    if(Gsm_Connect_Tcp_or_UdpPort(ip,port,DEFAULT_LINK_CHANNEL))
//    {
//            return CONNECT_ERR_CIPSTART;
//    }
//
//	
//	return CONNECT_ERR_NONE;
//}
//
///**************************************************************
// *发送数据
// *************************************************************/
//uint8_t Gsm_Send_data(uint8_t *s, uint32_t size)
//{ 
//	uint8_t inf[50];
//	uint8_t mux = GPRS_CIPMUX_TYPE;
//	uint8_t channel = DEFAULT_LINK_CHANNEL;
//	if(1==mux)
//	{
//		sprintf((char*)inf,"AT+CIPSEND=%d,%d\r\n",channel,size);	
//	}
//	else
//	{
//		sprintf((char*)inf,"AT+CIPSEND=%d\r\n",size);	
//	}	
//
//	
//	if(Gsm_SendAndWait(inf,(uint8_t *)">",RETRY_NUM,1))
//	{
//		return CONNECT_ERR_CIPSEND;	
//	}
//
//	SIM7600_SendData(s,size);	
//	
//	
//	Gsm_wait((uint8_t *)"SEND OK",1,4);
//	
//	return CONNECT_ERR_NONE;
//}	
//
//uint16_t Gsm_Recv_data(uint8_t* buf, uint16_t size)
//{
//    //OS_ERR      err;
//    uint8_t rst;
//    uint16_t len=0;
//    uint16_t cnlen = 0;
//    uint16_t offset = 0;
//    char *pst=NULL;
//    uint8_t inf[50];
//    uint8_t mode = 2;
//	uint8_t mux = GPRS_CIPMUX_TYPE;
//	uint8_t channel = DEFAULT_LINK_CHANNEL;
//	if(1==mux)
//		sprintf((char*)inf,"AT+CIPRXGET=%d,%d,%d\r\n",mode,channel,size);	
//	else
//		sprintf((char*)inf,"AT+CIPRXGET=%d,%d\r\n",mode,size);	   
//    do
//    {
//        rst = Gsm_SendAndWait(inf,(uint8_t *)"+CIPRXGET: 2",RETRY_NUM,1);
//        if(!rst)
//	    {
//            pst = strstr((char*)gprs_buf,"+CIPRXGET: 2");
//            pst = pst+15;
//            len = atoi(pst);
//            cnlen = atoi(pst+3);	        
//            if(len!=0)
//            {            
//                //pst = strstr(pst,"0x0A"); 
//                pst = strchr(pst,'\n');
//                memcpy(buf+offset,pst+1,len);                  
//                offset += len;
//            }    
//            vTaskDelay(pdMS_TO_TICKS(50));
//        }
//	}while((cnlen != 0)&&(rst == 1));
//	return offset;
//	
//}
//uint8_t Gsm_CloseConnect()
//{
//    Gsm_shutdowm_tcp_udp();
//    Gsm_shutdowm_socket();
//}