/*
 * ymodem.h
 *
 *  Created on: 2021. 1. 24.
 *      Author: baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_YMODEM_H_
#define SRC_COMMON_HW_INCLUDE_YMODEM_H_

//#include "hw_def.h"

#define  _USE_HW_YMODEM

#ifdef _USE_HW_YMODEM


#include <stdio.h>
#include <stdbool.h>
#include "usart.h"

#define YMODEM_BUF_LENGTH         (1024+5)

#define FLASH_ADDR_FW 	 0x8010000
//#define FLASH_


typedef enum
{
  YMODEM_TYPE_START,
  YMODEM_TYPE_DATA,
  YMODEM_TYPE_END,
  YMODEM_TYPE_CANCEL,
  YMODEM_TYPE_ERROR,
} YmodemType;


typedef struct
{
  uint8_t   state;
  uint16_t  index;

  uint8_t   stx;
  uint8_t   seq[2];
  uint8_t   *data;
  uint16_t  length;
  uint16_t  crc;
  uint16_t  crc_recv;
  uint8_t   buffer[YMODEM_BUF_LENGTH];
} ymodem_packet_t;


typedef struct
{
  uint8_t   ch;
  bool      is_init;
  uint32_t  baud;

  YmodemType type;
  uint8_t   rx_data;
  uint8_t   state;
  uint32_t  pre_time;
  uint32_t  start_time;
  uint32_t  index;
  uint8_t   error;
  uint8_t   ack_mode;

  char      file_name[128];
  uint32_t  file_addr;
  uint32_t  file_length;
  uint32_t  file_received;

  uint8_t  *file_buf;
  uint16_t  file_buf_length;


  ymodem_packet_t  rx_packet;
} ymodem_t;



bool ymodemInit(void);
bool ymodemOpen(ymodem_t *p_modem, uint8_t ch);
bool ymodemReceive(ymodem_t *p_modem);
bool ymodemAck(ymodem_t *p_modem);
#endif

#define TX_TIMEOUT          ((uint32_t)100)
#define RX_TIMEOUT          ((uint32_t)0xFFFFFFFF)

/* Exported macro ------------------------------------------------------------*/
#define IS_CAP_LETTER(c)    (((c) >= 'A') && ((c) <= 'F'))
#define IS_LC_LETTER(c)     (((c) >= 'a') && ((c) <= 'f'))
#define IS_09(c)            (((c) >= '0') && ((c) <= '9'))
#define ISVALIDHEX(c)       (IS_CAP_LETTER(c) || IS_LC_LETTER(c) || IS_09(c))
#define ISVALIDDEC(c)       IS_09(c)
#define CONVERTDEC(c)       (c - '0')

#define CONVERTHEX_ALPHA(c) (IS_CAP_LETTER(c) ? ((c) - 'A'+10) : ((c) - 'a'+10))
#define CONVERTHEX(c)       (IS_09(c) ? ((c) - '0') : CONVERTHEX_ALPHA(c))

/* Exported functions ------------------------------------------------------- */
void Int2Str(uint8_t *p_str, uint32_t intnum);
uint32_t Str2Int(uint8_t *inputstr, uint32_t *intnum);
void Serial_PutString(uint8_t *p_string);
HAL_StatusTypeDef Serial_PutByte(uint8_t param);


typedef enum
{
  COM_OK       = 0x00,
  COM_ERROR    = 0x01,
  COM_ABORT    = 0x02,
  COM_TIMEOUT  = 0x03,
  COM_DATA     = 0x04,
  COM_LIMIT    = 0x05
} COM_StatusTypeDef;
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/* Packet structure defines */
#define PACKET_HEADER_SIZE      ((uint32_t)3)
#define PACKET_DATA_INDEX       ((uint32_t)4)
#define PACKET_START_INDEX      ((uint32_t)1)
#define PACKET_NUMBER_INDEX     ((uint32_t)2)
#define PACKET_CNUMBER_INDEX    ((uint32_t)3)
#define PACKET_TRAILER_SIZE     ((uint32_t)2)
#define PACKET_OVERHEAD_SIZE    (PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE - 1)
#define PACKET_SIZE             ((uint32_t)128)
#define PACKET_1K_SIZE          ((uint32_t)1024)

/* /-------- Packet in IAP memory ------------------------------------------\
 * | 0      |  1    |  2     |  3   |  4      | ... | n+4     | n+5  | n+6  |
 * |------------------------------------------------------------------------|
 * | unused | start | number | !num | data[0] | ... | data[n] | crc0 | crc1 |
 * \------------------------------------------------------------------------/
 * the first byte is left unused for memory alignment reasons                 */

#define FILE_NAME_LENGTH        ((uint32_t)64)
#define FILE_SIZE_LENGTH        ((uint32_t)16)

#define SOH                     ((uint8_t)0x01)  /* start of 128-byte data packet */
#define STX                     ((uint8_t)0x02)  /* start of 1024-byte data packet */
#define EOT                     ((uint8_t)0x04)  /* end of transmission */
#define ACK                     ((uint8_t)0x06)  /* acknowledge */
#define NAK                     ((uint8_t)0x15)  /* negative acknowledge */
#define CA                      ((uint32_t)0x18) /* two of these in succession aborts transfer */
#define CRC16                   ((uint8_t)0x43)  /* 'C' == 0x43, request 16-bit CRC */
#define NEGATIVE_BYTE           ((uint8_t)0xFF)

#define ABORT1                  ((uint8_t)0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  ((uint8_t)0x61)  /* 'a' == 0x61, abort by user */

#define NAK_TIMEOUT             ((uint32_t)0x100000)
#define DOWNLOAD_TIMEOUT        ((uint32_t)10000) /* One second retry delay */
#define MAX_ERRORS              ((uint32_t)5)

/* Exported functions ------------------------------------------------------- */
COM_StatusTypeDef Ymodem_Receive(uint32_t *p_size);
COM_StatusTypeDef Ymodem_Transmit(uint8_t *p_buf, const uint8_t *p_file_name, uint32_t file_size);

#endif /* SRC_COMMON_HW_INCLUDE_YMODEM_H_ */
