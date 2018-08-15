/*
************************************************************************************************************************
*                                                        SoftI2C
*                                                   软件实现的I2C通信
*
* File     : SoftI2C.h
* Encoding : UTF-8
* By       : SXG
* Version  : V1.0
* Date     : 2016.05.17
************************************************************************************************************************
*/

#ifndef __I2C_H__
#define __I2C_H__

#include "stdint.h"


#define I2C_ERR_NONE             0x00      /* 无错误                       */
#define I2C_ERR_TIMEOUT          0x01      /* 超时(I2C总线被其它线程占用)  */
#define I2C_ERR_LINE             0x02      /* 总线错误                     */
#define I2C_ERR_UNKNOW           0xFF      /* 未知错误                     */


/// <summary>
/// I2C 初始化
/// </summary>
void    I2C_Init(void);

/// <summary>
/// I2C 数据传输
/// </summary>
/// <param name="ch_index">I2C索引号</param>
/// <param name="addr">从机地址</param>
/// <param name="tx_buf">发送缓冲区地址</param>
/// <param name="tx_len">发送字节数</param>
/// <param name="rx_buf">接收缓冲区地址</param>
/// <param name="rx_len">接收字节数</param>
/// <param name="retry_times">重试次数</param>
/// <returns>I2C_ERR_NONE:成功,其它值:失败</returns>
uint8_t I2C_Trans(uint8_t ch_index, uint8_t addr, uint8_t* tx_buf, uint16_t tx_len, uint8_t* rx_buf, uint16_t rx_len, uint8_t retry_times);




#endif /* __SOFTI2C_H__ */
