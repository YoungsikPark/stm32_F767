/*
 * flash.h
 *
 *  Created on: 2020. 12. 14.
 *      Author: baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_FLASH_H_
#define SRC_COMMON_HW_INCLUDE_FLASH_H_

//#include "hw_def.h"

#include <stdbool.h>
#include <stdio.h>

#define _USE_HW_FLASH 1

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) // Sector 0, 16 Kbytes
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) // Sector 1, 16 Kbytes
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) // Sector 2, 16 Kbytes
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) // Sector 3, 16 Kbytes
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) // Sector 4, 64 Kbytes
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) // Sector 5, 128 Kbytes
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) // Sector 6, 128 Kbytes
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) // Sector 7, 128 Kbytes
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) // Sector 8, 128 Kbytes
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) // Sector 9, 128 Kbytes
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) // Sector10, 128 Kbytes
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) // Sector11, 128 Kbytes
#define ADDR_FLASH_SECTOR_12    ((uint32_t)0x08100000) // Sector12, 128 Kbytes
#define ADDR_FLASH_SECTOR_13    ((uint32_t)0x08120000) // Sector13, 128 Kbytes
#define ADDR_FLASH_SECTOR_14    ((uint32_t)0x08140000) // Sector14, 128 Kbytes
#define ADDR_FLASH_SECTOR_15    ((uint32_t)0x08160000) // Sector15, 128 Kbytes

#ifdef _USE_HW_FLASH


extern bool flashInit(void);
extern bool flashErase(uint32_t addr, uint32_t length);
extern bool flashWrite(uint32_t addr, uint8_t *p_data, uint32_t length);
extern bool flashRead(uint32_t addr, uint8_t *p_data, uint32_t length);


#endif


#endif /* SRC_COMMON_HW_INCLUDE_FLASH_H_ */
