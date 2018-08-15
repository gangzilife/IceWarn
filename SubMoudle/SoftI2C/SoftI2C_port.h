#include  "stm32f4xx_hal.h"
#include  "FreeRTOS.h"
#include  "task.h"
#include  "semphr.h"

#ifndef __I2C_PORT_H__
#define __I2C_PORT_H__


// 需要多任务操作 I2C ?
// !!! 当配置为 0 时,禁止多个任务使用此模块 !!!
#define I2C_MULTI_TASK           0        /* 0: Disable  1: Enable        */          

#define I2C_CH_NB                1        /* I2C 通道数                   */
#define I2C_THIS_SPEED           400      /* I2C 通信速率,单位:kbps       */
#define I2C_RETRY_TIMES_DEFAULT  3        /* I2C 默认重试次数             */


#endif /* __SOFTI2C_PORT_H__ */
