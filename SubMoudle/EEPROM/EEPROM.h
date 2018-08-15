#ifndef __EEPROM_H__
#define __EEPROM_H__

#ifdef __cplusplus 
extern "C" {
#endif

#include "stdint.h"
#include "EEPROM_port.h"

#define EEPROM_ERR_NONE             0x00
#define EEPROM_ERR_TIMEOUT          0x01
#define EEPROM_ERR_ARGS             0x02
#define EEPROM_ERR_I2C              0x03

uint8_t EEPROM_Init(void);
uint8_t EEPROM_GetStatus(void);
uint8_t EEPROM_Write(uint32_t addr, const void* buf, uint16_t len);
uint8_t EEPROM_Read(uint32_t addr, void* buf, uint16_t len);


#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_H__ */