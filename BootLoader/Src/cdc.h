/*
 * cdc.h
 *
 *  Created on: 2020. 12. 11.
 *      Author: baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_CDC_H_
#define SRC_COMMON_HW_INCLUDE_CDC_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>



bool     cdcInit(void);
bool     cdcIsInit(void);
/*
uint32_t cdcAvailable(void);
uint8_t  cdcRead(void);
void     cdcDataIn(uint8_t rx_data);
uint32_t cdcWrite(uint8_t *p_data, uint32_t length);
uint32_t cdcGetBaud(void);
*/


#endif /* SRC_COMMON_HW_INCLUDE_CDC_H_ */
