#ifndef __EEPROM_PORT_H__
#define __EEPROM_PORT_H__

#ifdef __cplusplus 
extern "C" {
#endif

/*支持的设备列表
24C01(128 * 8) 128字节    
24C02(256 * 8) 256字节
24C04(512 * 8) 512字节
24C08(1024 * 8) 1K字节
24C16(2048 * 8) 2K字节
24C32(4096 * 8) 4K字节
24C64(8192 * 8) 8K字节
24C128(16384 * 8) 16K字节
24C256(32768 * 8) 32K字节
24C512(65536 * 8) 64K字节
*/
#define AT24C01             0
#define AT24C02             1
#define AT24C04             2
#define AT24C08             3
#define AT24C16             4
#define AT24C32             5
#define AT24C64             6
#define AT24C128            7
#define AT24C256            8
#define AT24C512            9

#define EEPROM_I2C_INDEX        (0)
#define EEPROM_SIZE             (8 * 1024 / 8)      // 24C08 (1024 * 8)
#define EEPROM_PAGE_SIZE        (16)                 // 16-byte Page
#define EEPROM_SLAVE_ADDR       (0xA0)
#define EEPROM_DATA_ADDR_BYTES  (1)
#define EEPROM_TYPE             (AT24C08)



#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_PORT_H__*/