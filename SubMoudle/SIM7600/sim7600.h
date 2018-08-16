#ifndef __SIM7600_H__
#define __SIM7600_H_

#include "bsp_usart.h"
#include <stdint.h>

#define  DEFAULT_LINK_CHANNEL           0  //默认多连接的通道
#define  DEFAULT_SENDDATA_BACKINF_MODE	1  //发送成功时显示'<',返回"send ok"
#define  DEFAULT_TANS_MODE          	0  //0  非透传模式    1透传模式

//------------------------底层接口重定义------------------------
//#define SIM900_SendStr(str)   USART1_Puts(str,strlen((char*)str));USART5_Puts(str,strlen((char*)str))

//#define	SIM900_SendData(pdata,size)	USART5_Puts(pdata,size)

//------------------------—-参数配置--------------------------	
#define  RETRY_NUM          3       //重发次数


#define GSM_INIT_OK	 		0
#define GSM_INIT_FAILURE	1
#define GSM_CONNECT_FAILURE	2


#define CONNECT_ERR_CIPMODE		0x01
#define CONNECT_ERR_CSTT		0x02
#define CONNECT_ERR_NETOPEN		0x03
#define CONNECT_ERR_CIFSR		0x04
#define CONNECT_ERR_CIPOPEN 	0x05
#define CONNECT_ERR_CIPSPRT		0x06
#define CONNECT_ERR_AT			0x07
#define CONNECT_ERR_ATE0		0x08
#define CONNECT_ERR_IPR			0x09
#define CONNECT_ERR_CMGF		0x10
#define CONNECT_ERR_CSCS		0x11
#define CONNECT_ERR_CLIP		0x12
#define CONNECT_ERR_CIPSTATUS	0x13
#define CONNECT_ERR_CIPSEND		0x14

#define CONNECT_ERR_CSDT		0x15
#define CONNECT_ERR_CSMINS		0x16

#define CONNECT_ERR_FTPGET		0x17

#define CONNECT_ERR_CREG		0x18
#define	CONNECT_ERR_CIPMUX	    0x19
#define CONNECT_ERR_CIPRXGET    0x20
#define CONNECT_ERR_CPSI        0x21

#define CONNECT_ERR_NONE		0x00

#define SIM7600_SendStr(str)            USART2_Tx(str,strlen((char*)str))
#define	SIM7600_SendData(pdata,size)	USART2_Tx(pdata,size)



//extern  uint8_t   Gsm_wait(uint8_t *strwait,uint8_t trynum,uint8_t timeout);
//extern  uint8_t   Gsm_set(uint8_t *cmd);
//extern  void      Gsm_csq(uint8_t* csq);
//
////TCP
extern  void     Gsm_TurnON(void);
////extern  void    Gsm_RecvInit(void);
//extern  uint8_t  Gsm_Init(void);
//extern	uint8_t  Gsm_Send_data(uint8_t *s, uint32_t size);
extern	uint8_t  Gsm_Connect_Server(uint8_t *ip ,uint32_t port);
//extern  uint16_t Gsm_Recv_data(uint8_t* buf, uint16_t size);
//extern  uint8_t  Gsm_CloseConnect(void);
////AT


			  
#endif