#include "SoftI2C_port.h"

#define _I2C_0_SCL_PORT   GPIOB
#define _I2C_0_SDA_PORT   GPIOB


#define _I2C_0_SCL_PIN    GPIO_PIN_6
#define _I2C_0_SDA_PIN    GPIO_PIN_7


//#define _I2C_1_SCL_PORT   GPIOB
//#define _I2C_1_SDA_PORT   GPIOB
//
//
//#define _I2C_1_SCL_PIN    GPIO_PIN_10
//#define _I2C_1_SDA_PIN    GPIO_PIN_11


/// <summary>
/// SCL、SDA IO口初始化
/// 应设置为开漏
/// 若有多个通道,此处将所有通道全部初始化
/// </summary>
void _i2c_io_init(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitTypeDef  gpio_init;
	
    gpio_init.Pin   = _I2C_0_SCL_PIN;
    gpio_init.Mode  = GPIO_MODE_OUTPUT_OD;
    gpio_init.Pull  = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(_I2C_0_SCL_PORT, &gpio_init);
	
    gpio_init.Pin   = _I2C_0_SDA_PIN;
    gpio_init.Mode  = GPIO_MODE_OUTPUT_OD;
    gpio_init.Pull  = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(_I2C_0_SDA_PORT, &gpio_init);
    
//    gpio_init.Pin   = _I2C_1_SCL_PIN;
//    gpio_init.Mode  = GPIO_MODE_OUTPUT_OD;
//    gpio_init.Pull  = GPIO_NOPULL;
//    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
//    HAL_GPIO_Init(_I2C_1_SCL_PORT, &gpio_init);
//	
//    gpio_init.Pin   = _I2C_1_SDA_PIN;
//    gpio_init.Mode  = GPIO_MODE_OUTPUT_OD;
//    gpio_init.Pull  = GPIO_NOPULL;
//    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
//    HAL_GPIO_Init(_I2C_1_SDA_PORT, &gpio_init);
}

/// <summary>
/// 将 SCL 拉低
/// </summary>
/// <param name="ch_index">I2C索引号</param>
void _i2c_scl_0(uint8_t ch_index)
{
	if (ch_index == 0)      HAL_GPIO_WritePin(_I2C_0_SCL_PORT, _I2C_0_SCL_PIN, GPIO_PIN_RESET);
    //if (ch_index == 1)      HAL_GPIO_WritePin(_I2C_1_SCL_PORT, _I2C_1_SCL_PIN, GPIO_PIN_RESET);
}

/// <summary>
/// 将 SCL 拉高
/// 将 SCL 切换到输入模式，由于上拉电阻的作用，SCL 即为高电平
/// ！！！ 切勿将 SCL 推挽输出高电平 ！！！
/// </summary>
/// <param name="ch_index">I2C索引号</param>
void _i2c_scl_1(uint8_t ch_index)
{
	if (ch_index == 0)      HAL_GPIO_WritePin(_I2C_0_SCL_PORT, _I2C_0_SCL_PIN, GPIO_PIN_SET);   
	//if (ch_index == 1)      HAL_GPIO_WritePin(_I2C_1_SCL_PORT, _I2C_1_SCL_PIN, GPIO_PIN_SET);
}

/// <summary>
/// 将 SDA 拉低
/// </summary>
/// <param name="ch_index">I2C索引号</param>
void _i2c_sda_0(uint8_t ch_index)
{
	if (ch_index == 0)      HAL_GPIO_WritePin(_I2C_0_SDA_PORT, _I2C_0_SDA_PIN, GPIO_PIN_RESET);
    //if (ch_index == 1)      HAL_GPIO_WritePin(_I2C_1_SDA_PORT, _I2C_1_SDA_PIN, GPIO_PIN_RESET);

}

/// <summary>
/// 将 SDA 拉高
/// 将 SDA 切换到输入模式，由于上拉电阻的作用，SDA 即为高电平
/// ！！！ 切勿将 SDA 推挽输出高电平 ！！！
/// </summary>
/// <param name="ch_index">I2C索引号</param>
void _i2c_sda_1(uint8_t ch_index)
{
	if (ch_index == 0)      HAL_GPIO_WritePin(_I2C_0_SDA_PORT, _I2C_0_SDA_PIN, GPIO_PIN_SET);
    //if (ch_index == 1)      HAL_GPIO_WritePin(_I2C_1_SDA_PORT, _I2C_1_SDA_PIN, GPIO_PIN_SET);
}

/// <summary>
/// 获取 SCL 电平值
/// </summary>
/// <param name="ch_index">I2C索引号</param>
/// <returns>0:低电平, 1:高电平</returns>
uint8_t _i2c_scl_is_high(uint8_t ch_index)
{
	if (ch_index == 0)      return (HAL_GPIO_ReadPin(_I2C_0_SCL_PORT, _I2C_0_SCL_PIN) == GPIO_PIN_SET);
	
    //if (ch_index == 1)      return (HAL_GPIO_ReadPin(_I2C_1_SCL_PORT, _I2C_1_SCL_PIN) == GPIO_PIN_SET);
	else return 0;
}

/// <summary>
/// 获取 SDA 电平值
/// </summary>
/// <param name="ch_index">I2C索引号</param>
/// <returns>0:低电平, 1:高电平</returns>
uint8_t _i2c_sda_is_high(uint8_t ch_index)
{
	if (ch_index == 0)      return (HAL_GPIO_ReadPin(_I2C_0_SDA_PORT, _I2C_0_SDA_PIN) == GPIO_PIN_SET);
	//if (ch_index == 1)      return (HAL_GPIO_ReadPin(_I2C_1_SDA_PORT, _I2C_1_SDA_PIN) == GPIO_PIN_SET);
	else return 0;
}

/// <summary>
/// I2C SCL延时函数(延时时间为半个SCL周期)
/// </summary>
void _i2c_dly_scl(void)
{
#if I2C_THIS_SPEED >= 400
	int16_t i = 5;
#elif I2C_THIS_SPEED >= 100
	int16_t i = 33;
#endif	
	while (i--)
	{
		__no_operation();
	}
}

/// <summary>
/// I2C 毫秒延时函数(延时时间为半个SCL周期)
/// </summary>
/// <param name="ms">毫秒数</param>
void _i2c_dly_ms(uint8_t ms)
{
	//OS_ERR err;
	//OSTimeDlyHMSM(0u, 0u, 0u, ms, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    vTaskDelay(ms);
}

#if I2C_MULTI_TASK > 0


static SemaphoreHandle_t _bus_mutex[I2C_CH_NB];

/// <summary>
/// I2C 多线程锁初始化
/// </summary>
void    _I2C_Lock_Init(void)
{
	for (uint8_t i = 0; i < I2C_CH_NB; i++)
	{
		//OSMutexCreate (&_bus_mutex[i], "I2C Lock Mutex", NULL);
        _bus_mutex[i] = xSemaphoreCreateMutex();
	}
}

/// <summary>
/// 进入临界区
/// </summary>
/// <param name="ch_index">I2C索引号</param>
/// <returns>1:OK, 0:Error</returns>
uint8_t _I2C_Lock(uint8_t ch_index)
{
//    OS_ERR err;
//	OSMutexPend(&_bus_mutex[ch_index], 1000, OS_OPT_PEND_BLOCKING, NULL, &err);
    if(xSemaphoreTake( _bus_mutex[ch_index], 1000 ) == pdTRUE)
        return 1;
    else
        return 0;
}

/// <summary>
/// 退出临界区
/// </summary>
/// <param name="ch_index">I2C索引号</param>
void    _I2C_Unlock(uint8_t ch_index)
{
	//OSMutexPost(&_bus_mutex[ch_index], OS_OPT_POST_NONE, NULL);
    xSemaphoreGive( _bus_mutex[ch_index] );
}

#endif /* SOFTI2C_MULTI_TASK > 0 */
