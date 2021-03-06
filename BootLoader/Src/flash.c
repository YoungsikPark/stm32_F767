/*
 * flash.c
 *
 *  Created on: 2020. 12. 14.
 *      Author: baram
 */


#include "flash.h"
#include "cli.h"

#include <stdio.h>
#include <stdbool.h>
#include "stm32f7xx_hal_def.h"
#include "stm32f7xx_hal_flash.h"

#ifdef _USE_HW_FLASH
#define DATA_32                 ((uint32_t)0x12345678)
#define FLASH_SECTOR_MAX          12
#define FLASH_USER_START_ADDR     0x08010000
#define FLASH_USER_END_ADDR       0x081FFFFF

FLASH_OBProgramInitTypeDef    OBInit;
HAL_StatusTypeDef status;
FLASH_EraseInitTypeDef init;

HAL_StatusTypeDef ReadFlash(void);
uint32_t GetWriteProtect(void);
HAL_StatusTypeDef EnableWriteProtect(void);
HAL_StatusTypeDef DisableWriteProtect(void);
//uint32_t GetSector(uint32_t Address);
uint32_t GetSectorSize(uint32_t Sector);

static uint32_t GetSector(uint32_t Address);

void cliFlash(cli_args_t *args);

typedef struct
{
  uint32_t addr;
  uint32_t length;
} flash_tbl_t;


static flash_tbl_t flash_tbl[FLASH_SECTOR_MAX]=
{
        {0x8000000,  32*1024},
        {0x8008000,  32*1024},
        {0x8010000,  32*1024},
        {0x8018000,  32*1024},
        {0x8020000, 128*1024},
        {0x8040000, 256*1024},
        {0x8080000, 256*1024},
    	{0x80C0000, 256*1024},
		{0x8100000, 256*1024},
		{0x8140000, 256*1024},
		{0x8180000, 256*1024},
		{0x81C0000, 256*1024},
};


static bool flashInSector(uint16_t sector_num, uint32_t addr, uint32_t length);



bool flashInit(void)
{
	/*
  for (int i=0; i<FLASH_SECTOR_MAX; i++)
  {
    flash_tbl[i].addr   = 0x8000000 + i*1024;
    flash_tbl[i].length = 1024;
  }*/

  //cliAdd("flash", cliFlash);

  return true;
}

bool flashErase(uint32_t addr, uint32_t length)
{
  bool ret = false;
  uint32_t page_error=0;
  uint32_t end_addr=0;
  int16_t  start_sector_num = -1;
  uint32_t sector_count = 0;

  uint32_t FirstSector = 0, NbOfSectors = 0;

    ret = HAL_FLASH_Unlock();
//    ret = HAL_FLASH_OB_Unlock();
    /* Get the Dual bank configuration status */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                             FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_ERSERR);

//    HAL_FLASHEx_OBGetConfig(&OBInit);

    end_addr = addr+length;

    /* Get the number of sector to erase from 1st sector*/
    FirstSector = GetSector(addr);
    NbOfSectors = GetSector(addr) - FirstSector + 1;

    init.TypeErase      = FLASH_TYPEERASE_SECTORS;
    init.VoltageRange   = FLASH_VOLTAGE_RANGE_3; // FLASH_BANK_1;
    init.Sector 		= FirstSector; // 지우기 시작할 페이지
    init.NbSectors      = NbOfSectors; // 지울 페이지 수

    status = HAL_FLASHEx_Erase(&init, &page_error);
    if (status == HAL_OK)
    {
      ret = true;
    }
    HAL_FLASH_Lock();

  return ret;
}

bool flashWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  HAL_StatusTypeDef status;
/*
  if(addr%2 != 0)
  {
	  return false;
  }
*/
  HAL_FLASH_Unlock();

  for (int i=0; i<length; i+=1) //i+2
  {
    uint16_t data;

    data  = p_data[i+0] << 0;
    //data |= p_data[i+1] << 8;

    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + i, (uint64_t)data);
    if (status != HAL_OK)
    {
      ret = false;
      break;
    }
  }

  HAL_FLASH_Lock();

  return ret;
}


uint32_t FLASH_If_Write(uint32_t FlashAddress, uint32_t* Data ,uint32_t DataLength)
{
  uint32_t i = 0;

  for (i = 0; (i < DataLength) && (FlashAddress <= (USER_FLASH_END_ADDRESS-4)); i++)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */
    if (HAL_FLASH_Program(TYPEPROGRAM_WORD, FlashAddress, *(uint32_t*)(Data+i)) == HAL_OK)
    {
     /* Check the written value */
      if (*(uint32_t*)FlashAddress != *(uint32_t*)(Data+i))
      {
        /* Flash content doesn't match SRAM content */
        return(FLASHIF_WRITINGCTRL_ERROR);
      }
      /* Increment FLASH destination address */
      FlashAddress += 4;
    }
    else
    {
      /* Error occurred while writing data in Flash memory */
      return (FLASHIF_WRITING_ERROR);
    }
  }

  return (FLASHIF_OK);
}

bool flashRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
//  uint8_t *p_byte[length];
  uint8_t *p_byte = (uint8_t *)addr;

  for (int i=0; i<length; i++)
  {
	printf("0x0%X : 0x%X\r\n",0x08000000+(180*1024)+i, p_byte[i]);
    p_data[i] = p_byte[i];
  }

  return ret;
}


HAL_StatusTypeDef ReadFlash()
{
  __IO uint8_t data8 = 0 , MemoryProgramStatus = 0;
  uint32_t Address = 0x08010000;

  while(Address < 0x08017fff)
  {
    data8 = *(__IO uint32_t*)Address;

    if(data8 != DATA_32){
      MemoryProgramStatus++;
    }
    Address = Address + 1;
    printf("0x0%X : 0x%X\r\n",Address,data8);
  }

  if(MemoryProgramStatus > 0){
    return HAL_ERROR;
  }
  return HAL_OK;
}

uint32_t GetWriteProtect()
{
  FLASH_OBProgramInitTypeDef OBInit;

  HAL_FLASHEx_OBGetConfig(&OBInit);
  uint32_t SectorsWRPStatus = OBInit.WRPSector;// & FLASH_WRP_SECTORS;

  return SectorsWRPStatus;
}

HAL_StatusTypeDef EnableWriteProtect()
{
  FLASH_OBProgramInitTypeDef OBInit;

  HAL_FLASH_OB_Unlock();
  HAL_FLASH_Unlock();

  OBInit.OptionType = OPTIONBYTE_WRP;
  OBInit.WRPState   = OB_WRPSTATE_ENABLE;
 // OBInit.Banks      = FLASH_BANK_1;
//  OBInit.WRPSector  = FLASH_WRP_SECTORS;
  HAL_FLASHEx_OBProgram(&OBInit);

  if (HAL_FLASH_OB_Launch() != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();

  return HAL_OK;
}

HAL_StatusTypeDef DisableWriteProtect()
{
  FLASH_OBProgramInitTypeDef OBInit;

  HAL_FLASH_OB_Unlock();
  HAL_FLASH_Unlock();

  OBInit.OptionType = OPTIONBYTE_WRP;
  OBInit.WRPState   = OB_WRPSTATE_DISABLE;
//  OBInit.Banks      = FLASH_BANK_1;
//  OBInit.WRPSector  = FLASH_WRP_SECTORS;

  HAL_FLASHEx_OBProgram(&OBInit);

  if (HAL_FLASH_OB_Launch() != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();

  return HAL_OK;
}

bool flashInSector(uint16_t sector_num, uint32_t addr, uint32_t length)
{
  bool ret = false;

  uint32_t sector_start;
  uint32_t sector_end;
  uint32_t flash_start;
  uint32_t flash_end;


  sector_start = flash_tbl[sector_num].addr;
  sector_end   = flash_tbl[sector_num].addr + flash_tbl[sector_num].length - 1;
  flash_start  = addr;
  flash_end    = addr + length - 1;


  if (sector_start >= flash_start && sector_start <= flash_end)
  {
    ret = true;
  }

  if (sector_end >= flash_start && sector_end <= flash_end)
  {
    ret = true;
  }

  if (flash_start >= sector_start && flash_start <= sector_end)
  {
    ret = true;
  }

  if (flash_end >= sector_start && flash_end <= sector_end)
  {
    ret = true;
  }

  return ret;
}

static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;
  }
#if defined(DUAL_BANK)
  else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
  {
    sector = FLASH_SECTOR_11;
  }
  else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
  {
    sector = FLASH_SECTOR_12;
  }
  else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
  {
    sector = FLASH_SECTOR_13;
  }
  else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
  {
    sector = FLASH_SECTOR_14;
  }
  else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
  {
    sector = FLASH_SECTOR_15;
  }
  else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
  {
    sector = FLASH_SECTOR_16;
  }
  else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
  {
    sector = FLASH_SECTOR_17;
  }
  else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
  {
    sector = FLASH_SECTOR_18;
  }
  else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
  {
    sector = FLASH_SECTOR_19;
  }
  else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
  {
    sector = FLASH_SECTOR_20;
  }
  else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
  {
    sector = FLASH_SECTOR_21;
  }
  else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
  {
    sector = FLASH_SECTOR_22;
  }
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23) */
  {
    sector = FLASH_SECTOR_23;
  }
#else
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11) */
  {
    sector = FLASH_SECTOR_11;
  }
#endif /* DUAL_BANK */
  return sector;
}



void cliFlash(cli_args_t *args)
{
  bool ret = false;



  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    for (int i=0; i<FLASH_SECTOR_MAX; i++)
    {
      cliPrintf("0x%X : %dKB\r\n", flash_tbl[i].addr, flash_tbl[i].length/1024);
    }

    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "read") == true)
  {
    uint32_t addr;
    uint32_t length;

    addr   = (uint32_t)args->getData(1);
    length = (uint32_t)args->getData(2);

    for (int i=0; i<length; i++)
    {
      cliPrintf("0x%X : 0x%X\r\n", addr+i, *((uint8_t *)(addr+i)));
    }

    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "erase") == true)
  {
    uint32_t addr;
    uint32_t length;

    addr   = (uint32_t)args->getData(1);
    length = (uint32_t)args->getData(2);

    if (flashErase(addr, length) == true)
    {
      cliPrintf("Erase OK\r\n");
    }
    else
    {
      cliPrintf("Erase Fail\r\n");
    }
  }

  if (args->argc == 3 && args->isStr(0, "write") == true)
  {
    uint32_t addr;
    uint32_t data;

    addr   = (uint32_t)args->getData(1);
    data   = (uint32_t)args->getData(2);

    if (flashWrite(addr, (uint8_t *)&data, 4) == true)
    {
      cliPrintf("Write OK\r\n");
    }
    else
    {
      cliPrintf("Write Fail\r\n");
    }

    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("flash info\r\n");
    cliPrintf("flash read  addr length\r\n");
    cliPrintf("flash erase addr length\r\n");
    cliPrintf("flash write addr data\r\n");
  }
}


int32_t DiagFlash_CheckImage(const uint32_t base)
{
	int32_t		i, ret = 0;
	int32_t		crc_head, crc_block, crc_all_block;
	uint32_t	addr;
	STImgHead	*pHead;

	pHead = (STImgHead *)base;

	crc_head = make_crc16(0u, (uint8_t *)pHead, sizeof(STImgHead) - sizeof(uint16_t));
	if((crc_head < 0) || ((uint16_t)crc_head != pHead->crc_header))
	{
		ret = -1;
	}

	if((ret == 0) && (pHead->block_count > CRC_BLOCK_COUNT))
	{
		ret = -1;
	}

	crc_all_block = 0;
	if(ret == 0)
	{
		for(i = 0; i < pHead->block_count; i++)
		{
			addr = base + sizeof(STImgHead) + (i * CRC_BLOCK_SIZE);
			crc_block = make_crc16(0, (uint8_t *)addr, CRC_BLOCK_SIZE);
			crc_all_block = make_crc16(crc_all_block, (uint8_t *)addr, CRC_BLOCK_SIZE);
			if((crc_all_block < 0) || (crc_block < 0) || ((uint16_t)crc_block != pHead->crc_block[i]))
			{
				ret = -1;
				break;
			}
		}

		if((crc_all_block < 0) || ((uint16_t)crc_all_block != pHead->crc_all_block))
		{
			ret = -1;
		}
	}

	return(ret);
}
#endif
