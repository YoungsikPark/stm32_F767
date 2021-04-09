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

#ifdef _USE_HW_FLASH


extern bool flashInit(void);
extern bool flashErase(uint32_t addr, uint32_t length);
extern bool flashWrite(uint32_t addr, uint8_t *p_data, uint32_t length);
extern bool flashRead(uint32_t addr, uint8_t *p_data, uint32_t length);


#endif


#endif /* SRC_COMMON_HW_INCLUDE_FLASH_H_ */
