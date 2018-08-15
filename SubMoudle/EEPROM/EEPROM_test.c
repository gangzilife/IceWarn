#include "lib_math.h"
#include "EEPROM.h"


#define TEMP_BUF_SIZE   (EEPROM_PAGE_SIZE * 4)
static uint8_t tx_buf[TEMP_BUF_SIZE];	
static uint8_t rx_buf[TEMP_BUF_SIZE];	

static void EEPROM_Test(void)
{
	EEPROM_Init();
	ELogDBG(ELOG_ID_SYS, DEF_TRUE, "EEPROM_Test");
	Math_Init();
	while (1)
	{
		sys_activate(SYS_MOD_TASK_SYS);
		
		uint16_t addr = Math_Rand() % EEPROM_SIZE;
		uint16_t len = Math_Rand() % (EEPROM_SIZE - addr + 1);
		if (len == 0)
		{
			continue;
		}
		if (len > TEMP_BUF_SIZE)
		{
			len = Math_Rand() % TEMP_BUF_SIZE + 1;
		}
		//ELogDBG(ELOG_ID_SYS, "Test %04X %04X", addr, len);
		
		for (uint16_t i = 0; i < len; i++)
		{
			tx_buf[i] = (uint8_t)Math_Rand();
			rx_buf[i] = (uint8_t)Math_Rand();
		}
		uint16_t first = Math_Rand() % len;
		
		EEPROM_Write(addr, tx_buf, first);
		EEPROM_Write(addr + first, tx_buf + first, len - first);
		
		first = Math_Rand() % len;
		EEPROM_Read(addr, rx_buf, first);
		EEPROM_Read(addr + first, rx_buf + first, len - first);
		
		uint16_t err_cnt = 0;
		for (uint16_t i = 0; i < len; i++)
		{
			if (tx_buf[i] != rx_buf[i])
			{		
				err_cnt++;
			}
		}
		if (err_cnt > 0)
		{
			ELogDBG(ELOG_ID_SYS, DEF_TRUE, "err_cnt = %d", err_cnt);
		}
	}
}
static void EEPROM_Test2(void)
{
	EEPROM_Init();
	
	ELogDBG(ELOG_ID_SYS, DEF_TRUE, "EEPROM_Test2");
	
	while (1)
	{
		ELogDBG(ELOG_ID_SYS, DEF_TRUE, "Write Chip Start...");
		Math_Init();
		uint16_t addr = 0;
		while (addr < EEPROM_SIZE)
		{
			sys_activate(SYS_MOD_TASK_SYS);
			uint16_t len = Math_Rand() % TEMP_BUF_SIZE + 1;		
			if (addr + len > EEPROM_SIZE)
				len = EEPROM_SIZE - addr;
			for (uint16_t i = 0; i < len; i++)
			{
				tx_buf[i] = (uint8_t)Math_Rand();
			}		
			EEPROM_Write(addr, tx_buf, len);
			addr += len;
		}
		ELogDBG(ELOG_ID_SYS, DEF_TRUE, "Write Chip Finish.");
		ELogDBG(ELOG_ID_SYS, DEF_TRUE, "Read Chip Start...");
		Math_Init();
		addr = 0;
		while (addr < EEPROM_SIZE)
		{
			sys_activate(SYS_MOD_TASK_SYS);
			uint16_t len = Math_Rand() % TEMP_BUF_SIZE + 1;		
			if (addr + len > EEPROM_SIZE)
				len = EEPROM_SIZE - addr;
			EEPROM_Read(addr, rx_buf, len);
			uint16_t err_cnt = 0;
			for (uint16_t i = 0; i < len; i++)
			{
				if (rx_buf[i] != (uint8_t)Math_Rand())
				{
					err_cnt++;
				}
			}
			if (err_cnt > 0)
			{
				ELogDBG(ELOG_ID_SYS, DEF_TRUE, "err_cnt = %d", err_cnt);
			}		
			addr += len;
		}
		ELogDBG(ELOG_ID_SYS, DEF_TRUE, "Read Chip Finish.");
	}
}