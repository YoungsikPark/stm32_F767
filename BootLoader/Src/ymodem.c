/*
 * ymodem.c
 *
 *  Created on: 2021. 1. 24.
 *      Author: baram
 */


#include "ymodem.h"
#include "usart.h"
#include "cli.h"
#include "flash.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define YMODEM_SOH    0x01 // 128 바이트 데이터
#define YMODEM_STX    0x02 // 1024 바이트 데이터
#define YMODEM_ACK    0x06 // 전송 시작, 다음패킷전
#define YMODEM_NACK   0x15 // 전송 에러, 재전송 요청
#define YMODEM_EOT    0x04 // 통신 종료
#define YMODEM_C      0x43 // 패킷 요청
#define YMODEM_CAN    0x18 //
//#define YMODEM_BS     0x08 //두파이 코드 전송인데.. 쓰는곳 없음 ㅠ




/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CRC16_F       /* activate the CRC16 integrity */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t flashdestination;
extern UART_HandleTypeDef huart3;

/* @note ATTENTION - please keep this variable 32bit alligned */
uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
uint8_t aFileName[FILE_NAME_LENGTH];
/* Private function prototypes -----------------------------------------------*/
static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length);
static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk);
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout);
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);

enum
{
  YMODEM_STATE_WAIT_HEAD,
  YMODEM_STATE_WAIT_FIRST,
  YMODEM_STATE_WAIT_DATA,
  YMODEM_STATE_WAIT_LAST,
  YMODEM_STATE_WAIT_END,
  YMODEM_STATE_WAIT_CANCEL,
};

enum
{
  YMODEM_PACKET_WAIT_FIRST,
  YMODEM_PACKET_WAIT_SEQ1,
  YMODEM_PACKET_WAIT_SEQ2,
  YMODEM_PACKET_WAIT_DATA,
  YMODEM_PACKET_WAIT_CRCH,
  YMODEM_PACKET_WAIT_CRCL,
};

enum
{
  YMODEM_RESP_NONE,
  YMODEM_RESP_ACK,
  YMODEM_RESP_ACK_C,
};


#ifdef _USE_HW_CLI
static void cliYmodem(cli_args_t *args);
#endif


static uint16_t crc16(uint8_t *data, uint16_t size);
static bool ymodemReceivePacket(ymodem_packet_t *p_packet, uint8_t data_in);





#if 0
bool ymodemInit(void)
{
#ifdef _USE_HW_CLI
  cliAdd("ymodem", cliYmodem);
#endif

  return true;
}

bool ymodemOpen(ymodem_t *p_modem, uint8_t ch)
{
  bool ret = true;

  p_modem->ch = ch;
  p_modem->is_init = true;

  p_modem->state           = YMODEM_STATE_WAIT_HEAD;
  p_modem->rx_packet.state = YMODEM_PACKET_WAIT_FIRST;
  p_modem->file_buf        = &p_modem->rx_packet.buffer[3];
  p_modem->file_buf_length = 0;
  p_modem->pre_time        = millis();
  p_modem->start_time      = 3000;
  p_modem->ack_mode        = 0;

  p_modem->rx_packet.data = &p_modem->rx_packet.buffer[3];

  return ret;
}

bool ymodemPutch(ymodem_t *p_modem, uint8_t data)
{
  bool ret = true;

  ret = uartWrite(p_modem->ch, &data, 1);

  return ret;
}

bool ymodemGetFileInfo(ymodem_t *p_modem)
{
  bool ret = true;
  bool valid;
  uint16_t size_i;

  valid = false;
  for (int i=0; i<128; i++)
  {
    p_modem->file_name[i] = p_modem->rx_packet.data[i];
    if (p_modem->file_name[i] == 0x00)
    {
      size_i = i + 1;
      valid = true;
      break;
    }
  }

  if (valid == true)
  {
    for (int i=size_i; i<128; i++)
    {
      if (p_modem->rx_packet.data[i] == 0x20)
      {
        p_modem->rx_packet.data[i] = 0x00;
        break;
      }
    }

    p_modem->file_length = (uint32_t)strtoul((const char * )&p_modem->rx_packet.data[size_i], (char **)NULL, (int) 0);
  }

  return ret;
}

bool ymodemAck(ymodem_t *p_modem)
{
  if (p_modem->ack_mode == YMODEM_RESP_ACK_C)
  {
    ymodemPutch(p_modem, YMODEM_ACK);
    ymodemPutch(p_modem, YMODEM_C);
  }

  if (p_modem->ack_mode == YMODEM_RESP_ACK)
  {
    ymodemPutch(p_modem, YMODEM_ACK);
  }

  p_modem->ack_mode = YMODEM_RESP_NONE;

  return true;
}

/*********************************************************************/
/* y modem 본격 적으로 실행이 되는 함수.
/*********************************************************************/

bool ymodemReceive(ymodem_t *p_modem)
{
  bool ret = false;
  bool update = false;
  uint32_t buf_length;


  if (p_modem->is_init != true)
  {
    p_modem->type = YMODEM_TYPE_ERROR;
    return true;
  }

  if (uartAvailable(p_modem->ch) > 0)
  {
    p_modem->rx_data = uartRead(p_modem->ch);
    update = true;
  }

  if (update == true && ymodemReceivePacket(&p_modem->rx_packet, p_modem->rx_data) == true)
  {
    //uartPrintf(_DEF_UART1, "RxPacket 0x%X\n", p_modem->rx_packet.stx);

    if (p_modem->state != YMODEM_STATE_WAIT_HEAD)
    {
      if (p_modem->rx_packet.stx == YMODEM_CAN)
      {
        p_modem->state = YMODEM_STATE_WAIT_CANCEL;
      }
    }
    switch(p_modem->state)
       {
         case YMODEM_STATE_WAIT_HEAD:
           if (p_modem->rx_packet.stx == YMODEM_EOT)
           {
             ymodemPutch(p_modem, YMODEM_NACK);
             p_modem->state = YMODEM_STATE_WAIT_LAST;
           }
           else if (p_modem->rx_packet.seq[0] == 0x00)
           {
             p_modem->file_addr = 0;
             ymodemGetFileInfo(p_modem);

             ymodemPutch(p_modem, YMODEM_ACK);
             ymodemPutch(p_modem, YMODEM_C);

             p_modem->state = YMODEM_STATE_WAIT_FIRST;
             p_modem->type = YMODEM_TYPE_START;
             ret = true;
           }
           break;

         case YMODEM_STATE_WAIT_FIRST:
           if (p_modem->rx_packet.stx == YMODEM_EOT)
           {
             ymodemPutch(p_modem, YMODEM_NACK);
             p_modem->state = YMODEM_STATE_WAIT_LAST;
           }
           else if (p_modem->rx_packet.seq[0] == 0x01)
           {
             p_modem->file_addr = 0;
             p_modem->file_received = 0;

             buf_length = (p_modem->file_length - p_modem->file_addr);
             if (buf_length > p_modem->rx_packet.length)
             {
               buf_length = p_modem->rx_packet.length;
             }
             p_modem->file_buf_length = buf_length;
             p_modem->file_received += buf_length;

             ymodemPutch(p_modem, YMODEM_ACK);

             p_modem->state = YMODEM_STATE_WAIT_DATA;
             p_modem->type = YMODEM_TYPE_DATA;
             ret = true;
           }
           break;

         case YMODEM_STATE_WAIT_DATA:
           if (p_modem->rx_packet.stx == YMODEM_EOT)
           {
             ymodemPutch(p_modem, YMODEM_NACK);
             p_modem->state = YMODEM_STATE_WAIT_LAST;
           }
           else
           {
             buf_length = (p_modem->file_length - p_modem->file_addr);
             if (buf_length > p_modem->rx_packet.length)
             {
               buf_length = p_modem->rx_packet.length;
             }
             p_modem->file_buf_length = buf_length;
             p_modem->file_addr += buf_length;
             p_modem->file_received += buf_length;

             ymodemPutch(p_modem, YMODEM_ACK);
             p_modem->type = YMODEM_TYPE_DATA;
             ret = true;
           }
           break;

         case YMODEM_STATE_WAIT_LAST:
           ymodemPutch(p_modem, YMODEM_ACK);
           ymodemPutch(p_modem, YMODEM_C);
           p_modem->state = YMODEM_STATE_WAIT_END;
           break;

         case YMODEM_STATE_WAIT_END:
           ymodemPutch(p_modem, YMODEM_ACK);
           p_modem->state = YMODEM_STATE_WAIT_HEAD;
           p_modem->type = YMODEM_TYPE_END;
           ret = true;
           break;

         case YMODEM_STATE_WAIT_CANCEL:
           ymodemPutch(p_modem, YMODEM_ACK);
           p_modem->state = YMODEM_STATE_WAIT_HEAD;
           p_modem->type = YMODEM_TYPE_CANCEL;
           ret = true;
           break;
       }
  }
  else
  {
    if (p_modem->rx_packet.state == YMODEM_PACKET_WAIT_FIRST)
    {
      if (millis()-p_modem->pre_time >= p_modem->start_time)
      {
        p_modem->pre_time = millis();
        ymodemPutch(p_modem, YMODEM_C);
      }
    }
  }
  return ret;
}

bool ymodemReceivePacket(ymodem_packet_t *p_packet, uint8_t data_in)
{
  bool ret = false;


  switch(p_packet->state)
  {
    case YMODEM_PACKET_WAIT_FIRST:
      if (data_in == YMODEM_SOH)
      {
        p_packet->length = 128;
        p_packet->stx = data_in;
        p_packet->state = YMODEM_PACKET_WAIT_SEQ1;
      }
      if (data_in == YMODEM_STX)
      {
        p_packet->length = 1024;
        p_packet->stx = data_in;
        p_packet->state = YMODEM_PACKET_WAIT_SEQ1;
      }
      if (data_in == YMODEM_EOT)
      {
        p_packet->stx = data_in;
        ret = true;
      }
      if (data_in == YMODEM_CAN)
      {
        p_packet->stx = data_in;
        ret = true;
      }
      break;

    case YMODEM_PACKET_WAIT_SEQ1:
      p_packet->seq[0] = data_in;
      p_packet->state = YMODEM_PACKET_WAIT_SEQ2;
      break;

    case YMODEM_PACKET_WAIT_SEQ2:
      p_packet->seq[1] = data_in;
      if (p_packet->seq[0] == (uint8_t)(~data_in))
      {
        p_packet->index = 0;
        p_packet->state = YMODEM_PACKET_WAIT_DATA;
      }
      else
      {
        p_packet->state = YMODEM_PACKET_WAIT_FIRST;
      }
      break;

    case YMODEM_PACKET_WAIT_DATA:
      p_packet->data[p_packet->index] = data_in;
      p_packet->index++;
      if (p_packet->index >= p_packet->length)
      {
        p_packet->state = YMODEM_PACKET_WAIT_CRCH;
      }
      break;

    case YMODEM_PACKET_WAIT_CRCH:
      p_packet->crc_recv = (data_in<<8);
      p_packet->state = YMODEM_PACKET_WAIT_CRCL;
      break;

    case YMODEM_PACKET_WAIT_CRCL:
      p_packet->crc_recv |= (data_in<<0);
      p_packet->state = YMODEM_PACKET_WAIT_FIRST;

      p_packet->crc = crc16(p_packet->data, p_packet->length);

      if (p_packet->crc == p_packet->crc_recv)
      {
        ret = true;
      }
      //uartPrintf(_DEF_UART1, "crc %X %X\n", p_packet->crc, p_packet->crc_recv);
      break;
  }
  return ret;
}

#define CRC_POLY 0x1021

uint16_t crc_update(uint16_t crc_in, int incr)
{
  uint16_t xor = crc_in >> 15;
  uint16_t out = crc_in << 1;

  if (incr)
  {
    out++;
  }

  if (xor)
  {
    out ^= CRC_POLY;
  }

  return out;
}

uint16_t crc16(uint8_t *data, uint16_t size)
{
  uint16_t crc, i;

  for (crc = 0; size > 0; size--, data++)
  {
    for (i = 0x80; i; i >>= 1)
    {
      crc = crc_update(crc, *data & i);
    }
  }

  for (i = 0; i < 16; i++)
  {
    crc = crc_update(crc, 0);
  }

  return crc;
}

void cliYmodem(cli_args_t *args)
{
  bool ret = false;
  ymodem_t ymodem;
  bool keep_loop = true;
  uint8_t err_code = 0;

  uint32_t pre_time;
  uint32_t addr_offset;
  uint32_t addr;

  uint32_t size = 0;
  COM_StatusTypeDef result;

  uint8_t log_ch = 1;

  memset(&ymodem,0,sizeof(ymodem_t));

  ymodemOpen(&ymodem, 2);
  cliPrintf("ymodem down [addr] \r\n");


  if (args->argc == 2 && args->isStr(0, "down"))
  {

    addr_offset = args->getData(1);

    cliPrintf("download ... 0x%x",addr_offset);

    while(keep_loop)
    {
      if (ymodemReceive(&ymodem) == true)
      {
        switch(ymodem.type)
        {
          case YMODEM_TYPE_START:
        	 uartPrintf(log_ch, "YMODEM_TYPE_START \r\n name : %s length : %d\r\n", ymodem.file_name, ymodem.file_length);
        	 if (flashErase(addr_offset, ymodem.file_length) != true)
			 {
			   keep_loop = false;
			   err_code = 1;
			 }
          case YMODEM_TYPE_DATA:
            uartPrintf(log_ch, "YMODEM_TYPE_DATA %d %d %%\r\n", ymodem.rx_packet.seq[0], ymodem.file_received*100 / ymodem.file_length);
           // uartPrintf(log_ch,"%x, %x ,%x\r\n",addr_offset + ymodem.file_addr,ymodem.file_buf, ymodem.file_buf_length);
		     if (flashWrite(addr_offset + ymodem.file_addr, ymodem.file_buf, ymodem.file_buf_length) != true)
		     {
			   keep_loop = false;
			   err_code = 2;
		     }
		    break;

          case YMODEM_TYPE_END:
            uartPrintf(log_ch, "YMODEM_TYPE_END \n");
            keep_loop = false;
            ret = true;
            break;

          case YMODEM_TYPE_CANCEL:
            uartPrintf(log_ch, "YMODEM_TYPE_CANCEL \n");
            err_code = 3;
            keep_loop = false;
            break;

          case YMODEM_TYPE_ERROR:
            uartPrintf(log_ch, "YMODEM_TYPE_ERROR \n");
            err_code = 4;
            keep_loop = false;
            break;
        }
         ymodemAck(&ymodem);

      }
    }
  }

    if( (ymodem.type == YMODEM_TYPE_END) || ((err_code != false)&&(ret == true)) )
    {
     cliPrintf("\r\nDown OK\r\n");
    }
    else
	{
	 cliPrintf("\r\nDown Fail ErrCode :%d \r\n",err_code);
	}
    return ret;
}


#endif



bool ymodemInit(void)
{
#ifdef _USE_HW_CLI
  cliAdd("ymodem", cliYmodem);
#endif

  return true;
}

bool ymodemOpen(ymodem_t *p_modem, uint8_t ch)
{
  bool ret = true;

  p_modem->ch = ch;
  p_modem->is_init = true;

  p_modem->state           = YMODEM_STATE_WAIT_HEAD;
  p_modem->rx_packet.state = YMODEM_PACKET_WAIT_FIRST;
  p_modem->file_buf        = &p_modem->rx_packet.buffer[3];
  p_modem->file_buf_length = 0;
  p_modem->pre_time        = millis();
  p_modem->start_time      = 3000;
  p_modem->ack_mode        = 0;

  p_modem->rx_packet.data = &p_modem->rx_packet.buffer[3];

  return ret;
}

bool ymodemPutch(ymodem_t *p_modem, uint8_t data)
{
  bool ret = true;

  ret = uartWrite(p_modem->ch, &data, 1);

  return ret;
}

bool ymodemGetFileInfo(ymodem_t *p_modem)
{
  bool ret = true;
  bool valid;
  uint16_t size_i;

  valid = false;
  for (int i=0; i<128; i++)
  {
    p_modem->file_name[i] = p_modem->rx_packet.data[i];
    if (p_modem->file_name[i] == 0x00)
    {
      size_i = i + 1;
      valid = true;
      break;
    }
  }

  if (valid == true)
  {
    for (int i=size_i; i<128; i++)
    {
      if (p_modem->rx_packet.data[i] == 0x20)
      {
        p_modem->rx_packet.data[i] = 0x00;
        break;
      }
    }

    p_modem->file_length = (uint32_t)strtoul((const char * )&p_modem->rx_packet.data[size_i], (char **)NULL, (int) 0);
  }

  return ret;
}

bool ymodemAck(ymodem_t *p_modem)
{
  if (p_modem->ack_mode == YMODEM_RESP_ACK_C)
  {
    ymodemPutch(p_modem, YMODEM_ACK);
    ymodemPutch(p_modem, YMODEM_C);
  }

  if (p_modem->ack_mode == YMODEM_RESP_ACK)
  {
    ymodemPutch(p_modem, YMODEM_ACK);
  }

  p_modem->ack_mode = YMODEM_RESP_NONE;

  return true;
}

bool ymodemReceive(ymodem_t *p_modem)
{
  bool ret = false;
  bool update = false;
  uint32_t buf_length;


  if (p_modem->is_init != true)
  {
    p_modem->type = YMODEM_TYPE_ERROR;
    return true;
  }

  if (uartAvailable(p_modem->ch) > 0)
  {
    p_modem->rx_data = uartRead(p_modem->ch);
    update = true;

   // uartPrintf(1, "Rx data: 0x%X\r\n", p_modem->rx_data);
   // uartPrintf(1, "Rx packet state: %d\r\n", p_modem->rx_packet.state);

  }

  if (update == true && ymodemReceivePacket(&p_modem->rx_packet, p_modem->rx_data) == true)
  {
    //uartPrintf(_DEF_UART1, "RxPacket 0x%X\n", p_modem->rx_packet.stx);

    if (p_modem->state != YMODEM_STATE_WAIT_HEAD)
    {
      if (p_modem->rx_packet.stx == YMODEM_CAN)
      {
        p_modem->state = YMODEM_STATE_WAIT_CANCEL;
      }
    }

    switch(p_modem->state)
    {
      case YMODEM_STATE_WAIT_HEAD:
        if (p_modem->rx_packet.stx == YMODEM_EOT)
        {
          ymodemPutch(p_modem, YMODEM_NACK);
          p_modem->state = YMODEM_STATE_WAIT_LAST;
        }
        else if (p_modem->rx_packet.seq[0] == 0x00)
        {
          p_modem->file_addr = 0;
          ymodemGetFileInfo(p_modem);

          //ymodemPutch(p_modem, YMODEM_ACK);
          //ymodemPutch(p_modem, YMODEM_C);
          p_modem->ack_mode = YMODEM_RESP_ACK_C;

          p_modem->state = YMODEM_STATE_WAIT_FIRST;
          p_modem->type = YMODEM_TYPE_START;
          ret = true;
        }
        break;

      case YMODEM_STATE_WAIT_FIRST:
        if (p_modem->rx_packet.stx == YMODEM_EOT)
        {
          ymodemPutch(p_modem, YMODEM_NACK);
          p_modem->state = YMODEM_STATE_WAIT_LAST;
        }
        else if (p_modem->rx_packet.seq[0] == 0x01)
        {
          p_modem->file_addr = 0;
          p_modem->file_received = 0;

          //uartPrintf(1, "first data: %d\r\n", p_modem->rx_data);
          buf_length = (p_modem->file_length - p_modem->file_addr);
          if (buf_length > p_modem->rx_packet.length)
          {
            buf_length = p_modem->rx_packet.length;
          }
          p_modem->file_buf_length = buf_length;
          p_modem->file_received += buf_length;

          ymodemPutch(p_modem, YMODEM_ACK);

          p_modem->state = YMODEM_STATE_WAIT_DATA;
          //p_modem->state = YMODEM_RESP_ACK;
          p_modem->type = YMODEM_TYPE_DATA;

          ret = true;
        }
        break;

      case YMODEM_STATE_WAIT_DATA:
        if (p_modem->rx_packet.stx == YMODEM_EOT)
        {
          ymodemPutch(p_modem, YMODEM_NACK);
          p_modem->state = YMODEM_STATE_WAIT_LAST;
        }
        else
        {
          buf_length = (p_modem->file_length - p_modem->file_addr);
          if (buf_length > p_modem->rx_packet.length)
          {
            buf_length = p_modem->rx_packet.length;
          }
          p_modem->file_buf_length = buf_length;
          p_modem->file_addr += buf_length;
          p_modem->file_received += buf_length;

          //ymodemPutch(p_modem, YMODEM_ACK);
          p_modem->ack_mode = YMODEM_RESP_ACK;
          p_modem->type = YMODEM_TYPE_DATA;
          ret = true;
        }
        break;

      case YMODEM_STATE_WAIT_LAST:
        ymodemPutch(p_modem, YMODEM_ACK);
        ymodemPutch(p_modem, YMODEM_C);
        p_modem->state = YMODEM_STATE_WAIT_END;
        break;

      case YMODEM_STATE_WAIT_END:
        ymodemPutch(p_modem, YMODEM_ACK);
        p_modem->state = YMODEM_STATE_WAIT_HEAD;
        p_modem->type = YMODEM_TYPE_END;
        ret = true;
        break;

      case YMODEM_STATE_WAIT_CANCEL:
        ymodemPutch(p_modem, YMODEM_ACK);
        p_modem->state = YMODEM_STATE_WAIT_HEAD;
        p_modem->type = YMODEM_TYPE_CANCEL;
        ret = true;
        break;
    }
  }
  else
  {
	  //uartPrintf(1, "C");
    if (p_modem->rx_packet.state == YMODEM_PACKET_WAIT_FIRST)
    {
      if (millis()-p_modem->pre_time >= p_modem->start_time)
      {
        p_modem->pre_time = millis();
        ymodemPutch(p_modem, YMODEM_C);

      }
    }
  }

//  uartPrintf(1,"Return : %d\r\n",ret);
  return ret;
}

bool ymodemReceivePacket(ymodem_packet_t *p_packet, uint8_t data_in)
{
  bool ret = false;


  switch(p_packet->state)
  {
    case YMODEM_PACKET_WAIT_FIRST:
      if (data_in == YMODEM_SOH)
      {
        p_packet->length = 128;
        p_packet->stx = data_in;
        p_packet->state = YMODEM_PACKET_WAIT_SEQ1;
      }
      if (data_in == YMODEM_STX)
      {
        p_packet->length = 1024;
        p_packet->stx = data_in;
        p_packet->state = YMODEM_PACKET_WAIT_SEQ1;
      }
      if (data_in == YMODEM_EOT)
      {
        p_packet->stx = data_in;
        ret = true;
      }
      if (data_in == YMODEM_CAN)
      {
        p_packet->stx = data_in;
        ret = true;
      }
      break;

    case YMODEM_PACKET_WAIT_SEQ1:
      p_packet->seq[0] = data_in;
      p_packet->state = YMODEM_PACKET_WAIT_SEQ2;
      break;

    case YMODEM_PACKET_WAIT_SEQ2:
      p_packet->seq[1] = data_in;
      if (p_packet->seq[0] == (uint8_t)(~data_in))
      {
        p_packet->index = 0;
        p_packet->state = YMODEM_PACKET_WAIT_DATA;
      }
      else
      {
        p_packet->state = YMODEM_PACKET_WAIT_FIRST;
      }
      break;

    case YMODEM_PACKET_WAIT_DATA:
      p_packet->data[p_packet->index] = data_in;
      p_packet->index++;
      if (p_packet->index >= p_packet->length)
      {
        p_packet->state = YMODEM_PACKET_WAIT_CRCH;
      }
      break;

    case YMODEM_PACKET_WAIT_CRCH:
      p_packet->crc_recv = (data_in<<8);
      p_packet->state = YMODEM_PACKET_WAIT_CRCL;
      break;

    case YMODEM_PACKET_WAIT_CRCL:
      p_packet->crc_recv |= (data_in<<0);
      p_packet->state = YMODEM_PACKET_WAIT_FIRST;

      p_packet->crc = crc16(p_packet->data, p_packet->length);

      if (p_packet->crc == p_packet->crc_recv)
      {
        ret = true;
      }
      //uartPrintf(_DEF_UART1, "crc %X %X\n", p_packet->crc, p_packet->crc_recv);
      break;
  }

  return ret;
}


#define CRC_POLY 0x1021

uint16_t crc_update(uint16_t crc_in, int incr)
{
  uint16_t xor = crc_in >> 15;
  uint16_t out = crc_in << 1;

  if (incr)
  {
    out++;
  }

  if (xor)
  {
    out ^= CRC_POLY;
  }

  return out;
}

uint16_t crc16(uint8_t *data, uint16_t size)
{
  uint16_t crc, i;

  for (crc = 0; size > 0; size--, data++)
  {
    for (i = 0x80; i; i >>= 1)
    {
      crc = crc_update(crc, *data & i);
    }
  }

  for (i = 0; i < 16; i++)
  {
    crc = crc_update(crc, 0);
  }

  return crc;
}

  void cliYmodem(cli_args_t *args)
  {
    bool ret = false;
    ymodem_t ymodem;
    bool keep_loop = true;
    uint8_t err_code = 0;

    uint32_t pre_time;
    uint32_t addr_offset;
    uint32_t addr;

    uint32_t size = 0;
    COM_StatusTypeDef result;

    uint8_t log_ch = 1;

    memset(&ymodem,0,sizeof(ymodem_t));

    ymodemOpen(&ymodem, 2);
    //cliPrintf("ymodem down [addr] \r\n");


    if (args->argc == 1 && args->isStr(0, "down"))
    {

      addr_offset = 0x8010000;//args->getData(1);

      cliPrintf("download ... 0x%x",addr_offset);

      while(keep_loop)
      {
        if (ymodemReceive(&ymodem) == true)
        {
          switch(ymodem.type)
          {
            case YMODEM_TYPE_START:
          	 uartPrintf(log_ch, "\r\nYMODEM_TYPE_START \r\n name : %s length : %d\r\n", ymodem.file_name, ymodem.file_length);
          	 if (flashErase(addr_offset, ymodem.file_length) != true)
  			 {
  			   keep_loop = false;
  			   err_code = 1;
  			 }
            case YMODEM_TYPE_DATA:
              uartPrintf(log_ch, "YMODEM_TYPE_DATA %d %d %%\r\n", ymodem.rx_packet.seq[0], ymodem.file_received*100 / ymodem.file_length);
              uartPrintf(log_ch,"%x, %x ,%x\r\n",addr_offset + ymodem.file_addr,ymodem.file_buf, ymodem.file_buf_length);
  		     if (flashWrite(addr_offset + ymodem.file_addr, ymodem.file_buf, ymodem.file_buf_length) != true)
  		     {
  			   keep_loop = false;
  			   err_code = 2;
  		     }
  		    break;

            case YMODEM_TYPE_END:
              uartPrintf(log_ch, "YMODEM_TYPE_END \n");
              keep_loop = false;
              ret = true;
              break;

            case YMODEM_TYPE_CANCEL:
              uartPrintf(log_ch, "YMODEM_TYPE_CANCEL \n");
              err_code = 3;
              keep_loop = false;
              break;

            case YMODEM_TYPE_ERROR:
              uartPrintf(log_ch, "YMODEM_TYPE_ERROR \n");
              err_code = 4;
              keep_loop = false;
              break;
          }
           ymodemAck(&ymodem);

        }
      }
    }

      if( (ymodem.type == YMODEM_TYPE_END) || ((err_code != false)&&(ret == true)) )
      {
       cliPrintf("\r\nDown OK\r\n");
      }
      else
  	{
  	 cliPrintf("\r\nDown Fail ErrCode :%d \r\n",err_code);
  	}
      return ret;
  }



