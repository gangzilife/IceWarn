#include "string.h"
#include "EEPROM.h"
#include "EEPROM_port.h"
#include "SoftI2C.h"

static uint8_t status = EEPROM_ERR_NONE;

uint8_t EEPROM_Init(void)
{
	I2C_Init();
	return EEPROM_ERR_NONE;
}

uint8_t EEPROM_GetStatus(void)
{
	return status;
}


#define MIN(a,b) ((a) < (b) ? (a) : (b))

uint8_t EEPROM_Write(uint32_t addr, const void* buf, uint16_t len)
{
	if ((buf == NULL) ||
	    (addr + len) > EEPROM_SIZE)
	{
		return EEPROM_ERR_ARGS;
	}
	
	uint8_t* p_data_buf = (uint8_t*)buf;
	uint16_t need_len = len;
	uint32_t w_addr = addr;

	uint8_t  tx_buf[EEPROM_DATA_ADDR_BYTES + EEPROM_PAGE_SIZE];		
	
	while (need_len > 0)
	{
        if(EEPROM_TYPE > AT24C16) //采用2个字节寻址，没有页地址，只有器件地址
        {
            // fill addr
            for (uint8_t i = 0; i < EEPROM_DATA_ADDR_BYTES; i++)
            {
                tx_buf[EEPROM_DATA_ADDR_BYTES - i - 1] = (uint8_t)(w_addr >> (8 * i));
            }
        }
        else //采用一个字节寻址，器件地址带有页地址
            tx_buf[EEPROM_DATA_ADDR_BYTES - 1] = (uint8_t)(w_addr % 256);
    

		// fill data
		uint16_t tx_data_len;
		if (w_addr % EEPROM_PAGE_SIZE) // not page aligned
		{
			tx_data_len = MIN(need_len, EEPROM_PAGE_SIZE - (w_addr % EEPROM_PAGE_SIZE));
		}
		else
		{
			tx_data_len = MIN(need_len, EEPROM_PAGE_SIZE);
		}
		memcpy(tx_buf + EEPROM_DATA_ADDR_BYTES, p_data_buf, tx_data_len);

        uint8_t err ;
        if(EEPROM_TYPE > AT24C16)
            err = I2C_Trans(EEPROM_I2C_INDEX, EEPROM_SLAVE_ADDR, tx_buf, EEPROM_DATA_ADDR_BYTES + tx_data_len , NULL, 0, 3);
        else
            err = I2C_Trans(EEPROM_I2C_INDEX, EEPROM_SLAVE_ADDR + ((w_addr/256)<<1), tx_buf, EEPROM_DATA_ADDR_BYTES + tx_data_len , NULL, 0, 3);
		status = err;
        if (err != I2C_ERR_NONE)
		{
			return EEPROM_ERR_I2C;
		}

		p_data_buf += tx_data_len;
		need_len -= tx_data_len;
		w_addr += tx_data_len;
	}
	return EEPROM_ERR_NONE;
}

uint8_t EEPROM_Read(uint32_t addr, void* buf, uint16_t len)
{
	if ((buf == NULL) ||
	    (addr + len) > EEPROM_SIZE)
	{
		return EEPROM_ERR_ARGS;
	}
    uint8_t  tx_buf[EEPROM_DATA_ADDR_BYTES];
    if(EEPROM_TYPE > AT24C16) //采用2个字节寻址，没有页地址，只有器件地址
    {
        // fill addr
        for (uint8_t i = 0; i < EEPROM_DATA_ADDR_BYTES; i++)
        {
            tx_buf[EEPROM_DATA_ADDR_BYTES - i - 1] = (uint8_t)(addr >> (8 * i));
        }
    }
    else //采用一个字节寻址，器件地址带有页地址
        tx_buf[EEPROM_DATA_ADDR_BYTES - 1] = (uint8_t)(addr % 256);
    
	uint8_t err ;
    if(EEPROM_TYPE > AT24C16)
        err = I2C_Trans(EEPROM_I2C_INDEX, EEPROM_SLAVE_ADDR, tx_buf, EEPROM_DATA_ADDR_BYTES, (uint8_t*)buf, len, 3);
    else
        err = I2C_Trans(EEPROM_I2C_INDEX, EEPROM_SLAVE_ADDR + ((addr/256)<<1), tx_buf, EEPROM_DATA_ADDR_BYTES, (uint8_t*)buf, len, 3);
        
	status = err;
	return (err == I2C_ERR_NONE) ? EEPROM_ERR_NONE : EEPROM_ERR_I2C;
}