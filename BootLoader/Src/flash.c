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
#define FLASH_SECTOR_MAX          8
#define FLASH_USER_START_ADDR     0x08008000
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


static flash_tbl_t flash_tbl[FLASH_SECTOR_MAX];
 /*{
        {0x8000000,  16*1024},
        {0x8004000,  16*1024},
        {0x8008000,  16*1024},
        {0x800C000,  16*1024},
        {0x8010000,  64*1024},
        {0x8020000, 128*1024},
        {0x8040000, 128*1024},
        {0x8060000, 128*1024},
    };
*/

static bool flashInSector(uint16_t sector_num, uint32_t addr, uint32_t length);



bool flashInit(void)
{
  for (int i=0; i<FLASH_SECTOR_MAX; i++)
  {
    flash_tbl[i].addr   = 0x8000000 + i*1024;
    flash_tbl[i].length = 1024;
  }

  cliAdd("flash", cliFlash);

  return true;
}

bool flashErase(uint32_t addr, uint32_t length)
{
  bool ret = false;
  uint32_t page_error=0;

  int16_t  start_sector_num = -1;
  uint32_t sector_count = 0;

  uint32_t FirstSector = 0, NbOfSectors = 0;


#if 0
  for (int i=0; i<FLASH_SECTOR_MAX; i++)
  {
    if (flashInSector(i, addr, length) == true)
    {
      if (start_sector_num < 0)
      {
        start_sector_num = i;
      }
      sector_count++;
    }
  }

   FirstSector = GetSector(FLASH_USER_START_ADDR);
   /* Get the number of sector to erase from 1st sector*/
   NbOfSectors = GetSector(FLASH_USER_END_ADDR) - FirstSector + 1;
   /* Fill EraseInit structure*/

   EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
   EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
   EraseInitStruct.Sector        = FirstSector;
   EraseInitStruct.NbSectors     = NbOfSectors;
#endif

 // if (sector_count > 0)
  //{

    ret = HAL_FLASH_Unlock();
    ret = HAL_FLASH_OB_Unlock();
    /* Get the Dual bank configuration status */
    HAL_FLASHEx_OBGetConfig(&OBInit);

    if((OBInit.USERConfig & OB_NDBANK_SINGLE_BANK) == OB_NDBANK_DUAL_BANK)
    {
  	  printf("Single Bank Flash init Ok\r\n");
    }

    /* Get the number of sector to erase from 1st sector*/

    FirstSector = GetSector(addr);
    NbOfSectors = GetSector(addr) - FirstSector + 1;

    printf("FirstSector : %d    ",FirstSector);
    printf("NbOfSectors : %d\r\n",NbOfSectors);

    init.TypeErase      = FLASH_TYPEERASE_SECTORS;
    init.VoltageRange   = FLASH_VOLTAGE_RANGE_3; // FLASH_BANK_1;
    init.Sector 		= FirstSector; // 지우기 시작할 페이지
    init.NbSectors      = NbOfSectors; // 지울 페이지 수

    status = HAL_FLASHEx_Erase(&init, &page_error);
    if (status == HAL_OK)
    {
      ret = true;
    }
/*
    __HAL_FLASH_DATA_CACHE_DISABLE();
    __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

    __HAL_FLASH_DATA_CACHE_RESET();
    __HAL_FLASH_INSTRUCTION_CACHE_RESET();

    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
    __HAL_FLASH_DATA_CACHE_ENABLE();
*/
    HAL_FLASH_Lock();
//  }

  return ret;
}

bool flashWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  HAL_StatusTypeDef status;


  if (addr%2 != 0)
  {
    return false;
  }

  HAL_FLASH_Unlock();

  for (int i=0; i<length; i+=2)
  {
    uint16_t data;

    data  = p_data[i+0] << 0;
    data |= p_data[i+1] << 8;

    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + i, (uint64_t)data);
    if (status != HAL_OK)
    {
      ret = false;
      break;
    }
  }

  HAL_FLASH_Lock();

  return ret;
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
#endif
