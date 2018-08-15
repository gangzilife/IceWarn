/*
************************************************************************************************************************
*                                                        SoftI2C
*                                                   软件实现的I2C通信
*
* File     : SoftI2C.c
* Encoding : UTF-8
* By       : SXG
* Version  : V1.0
* Date     : 2016.05.17
************************************************************************************************************************
*/

#include <stdbool.h>
#include <intrinsics.h>
#include "SoftI2C.h"
#include "SoftI2C_port.h"



// 在 SoftI2C_port.c 中实现的函数
void    _i2c_io_init(void);
void    _i2c_scl_0(uint8_t ch_index);
void    _i2c_scl_1(uint8_t ch_index);
void    _i2c_sda_0(uint8_t ch_index);
void    _i2c_sda_1(uint8_t ch_index);
uint8_t _i2c_scl_is_high(uint8_t ch_index);
uint8_t _i2c_sda_is_high(uint8_t ch_index);
void    _i2c_dly_scl(void);
void    _i2c_dly_ms(uint8_t ms);



#if I2C_MULTI_TASK == 0

void _I2C_Lock_Init() {}
uint8_t _I2C_Lock(uint8_t index) { return 1; }
void _I2C_Unlock(uint8_t index) {}

#else

void    _I2C_Lock_Init(void);
uint8_t _I2C_Lock(uint8_t ch_index);
void    _I2C_Unlock(uint8_t ch_index);

#endif /* I2C_MULTI_TASK == 0 */


#define _LOCK_I2C(index)    if (!_I2C_Lock(index)) { return I2C_ERR_TIMEOUT; }
#define _UNLOCK_I2C(index)  _I2C_Unlock(index);



void I2C_Init(void)
{
	static uint8_t is_called = 0;
	if (is_called)
		return;
	is_called = 1;

	_i2c_io_init();
	
	for (uint8_t i = 0; i < I2C_CH_NB; i++)
	{
		_i2c_scl_1(i);
		_i2c_sda_1(i);
	}
	_I2C_Lock_Init();
}

static void _reset(uint8_t ch_index)
{
	for (uint8_t i = 0; i < 10; i++)
	{
		_i2c_scl_0(ch_index);
		_i2c_dly_scl();
		_i2c_scl_1(ch_index);
		_i2c_dly_scl();
	}
}


static void _stop(uint8_t ch_index)
{
	_i2c_sda_0(ch_index);
	_i2c_dly_scl();
	_i2c_scl_1(ch_index);
	_i2c_dly_scl();
	_i2c_sda_1(ch_index);
	_i2c_dly_scl();
}

static uint8_t _start(uint8_t ch_index)
{
	_i2c_sda_1(ch_index);
	_i2c_scl_1(ch_index);
	_i2c_dly_scl();
	
	uint8_t retry = 10;
	while ((!_i2c_sda_is_high(ch_index) || !_i2c_scl_is_high(ch_index)) &&
		   (--retry))
	{
		_stop(ch_index);
		_reset(ch_index);
	}
	if (retry > 0)
	{
		_i2c_sda_0(ch_index);
		_i2c_dly_scl();
		_i2c_scl_0(ch_index);
		_i2c_dly_scl();
		return true;
	}
	else
	{
		return false;
	}
}


static uint8_t _tx_byte(uint8_t ch_index, uint8_t b)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if (b & 0x80)
			_i2c_sda_1(ch_index);
		else
			_i2c_sda_0(ch_index);
		b <<= 1;
		_i2c_scl_1(ch_index);
		_i2c_dly_scl();
		_i2c_scl_0(ch_index);
		_i2c_dly_scl();
	}
	_i2c_sda_1(ch_index);
	_i2c_dly_scl();
	_i2c_scl_1(ch_index);
	__no_operation();
	uint8_t is_ack = !_i2c_sda_is_high(ch_index);
	_i2c_scl_0(ch_index);
	_i2c_dly_scl();
	
	if (is_ack)
		return true;
	else
		return false;
}

static void _ack(uint8_t ch_index)
{
	_i2c_sda_0(ch_index);
	_i2c_dly_scl();
	_i2c_scl_1(ch_index);
	_i2c_dly_scl();
	_i2c_scl_0(ch_index);
	_i2c_dly_scl();
}

static uint8_t _rx_byte(uint8_t ch_index)
{
	uint8_t b = 0;
	_i2c_sda_1(ch_index);
	for (uint8_t i = 0; i < 8; i++)
	{
		_i2c_scl_1(ch_index);
		_i2c_dly_scl();
		b <<= 1;
		if (_i2c_sda_is_high(ch_index))
			b |= 0x01;
		_i2c_scl_0(ch_index);
		_i2c_dly_scl();
	}
	return b;
}


#define LINE_ERR_EXIT(index) {  _UNLOCK_I2C(index) return I2C_ERR_LINE; }

// 私有函数(单次传输)
static uint8_t _I2C_Trans(uint8_t ch_index, uint8_t addr, uint8_t* tx_buf, uint16_t tx_len, uint8_t* rx_buf, uint16_t rx_len)
{
	uint8_t is_success = 0;
	_LOCK_I2C(ch_index)
	if (tx_len > 0)
	{
		is_success = _start(ch_index);
		if (!is_success) LINE_ERR_EXIT(ch_index)
		is_success = _tx_byte(ch_index, addr);
		if (!is_success) LINE_ERR_EXIT(ch_index)
	}
	for (uint16_t i = 0; i < tx_len; i++)
	{
		is_success = _tx_byte(ch_index, tx_buf[i]);
		if (!is_success && rx_len > 0) LINE_ERR_EXIT(ch_index)
	}
	if (rx_len > 0)
	{
		is_success = _start(ch_index);
		if (!is_success) LINE_ERR_EXIT(ch_index)
		is_success = _tx_byte(ch_index, addr | 0x01);
		if (!is_success) LINE_ERR_EXIT(ch_index)
	}
	for (uint16_t i = 0; i < rx_len; i++)
	{
		rx_buf[i] = _rx_byte(ch_index);
		if (i != (rx_len - 1))
			_ack(ch_index);
	}
	_stop(ch_index);
	_i2c_dly_ms(10);
	_UNLOCK_I2C(ch_index)
		
	return I2C_ERR_NONE;
}

uint8_t I2C_Trans(uint8_t ch_index, uint8_t addr, uint8_t* tx_buf, uint16_t tx_len, uint8_t* rx_buf, uint16_t rx_len, uint8_t retry_times)
{
	uint8_t err = 0;
	uint8_t retry = I2C_RETRY_TIMES_DEFAULT;
	if (retry_times > 0 && retry_times < 100)
		retry = retry_times;
	while (retry--)
	{
		err = _I2C_Trans(ch_index, addr, tx_buf, tx_len, rx_buf, rx_len);
		if (err == I2C_ERR_LINE)
		{
			_i2c_dly_ms(10);
			_reset(ch_index);
		}
		else
		{
			break;
		}
	}

	return err;
}


