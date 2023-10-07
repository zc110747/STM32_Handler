//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_flash.c
//
//  Purpose:
//      flash manage driver.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DRV_FLASH_H
#define _DRV_FLASH_H

#include "main.h"

//bank 0
#define FLASH_SECTOR_START_ADDRESS_0    0x08000000      //16KB
#define FLASH_SECTOR_START_ADDRESS_1    0x08004000      //16KB
#define FLASH_SECTOR_START_ADDRESS_2    0x08008000      //16KB
#define FLASH_SECTOR_START_ADDRESS_3    0x0800C000      //16KB
#define FLASH_SECTOR_START_ADDRESS_4    0x08010000      //64KB
#define FLASH_SECTOR_START_ADDRESS_5    0x08020000      //128KB
#define FLASH_SECTOR_START_ADDRESS_6    0x08040000      //128KB
#define FLASH_SECTOR_START_ADDRESS_7    0x08060000      //128KB
#define FLASH_SECTOR_START_ADDRESS_8    0x08080000      //128KB
#define FLASH_SECTOR_START_ADDRESS_9    0x080a0000      //128KB
#define FLASH_SECTOR_START_ADDRESS_10   0x080c0000      //128KB
#define FLASH_SECTOR_START_ADDRESS_11   0x080e0000      //128KB

//bank 1
#define FLASH_SECTOR_START_ADDRESS_12   0x08100000      //16KB
#define FLASH_SECTOR_START_ADDRESS_13   0x08104000      //16KB
#define FLASH_SECTOR_START_ADDRESS_14   0x08108000      //16KB
#define FLASH_SECTOR_START_ADDRESS_15   0x0810C000      //16KB
#define FLASH_SECTOR_START_ADDRESS_16   0x08110000      //64KB
#define FLASH_SECTOR_START_ADDRESS_17   0x08120000      //128KB
#define FLASH_SECTOR_START_ADDRESS_18   0x08140000      //128KB
#define FLASH_SECTOR_START_ADDRESS_19   0x08160000      //128KB
#define FLASH_SECTOR_START_ADDRESS_20   0x08180000      //128KB
#define FLASH_SECTOR_START_ADDRESS_21   0x081a0000      //128KB
#define FLASH_SECTOR_START_ADDRESS_22   0x081c0000      //128KB
#define FLASH_SECTOR_START_ADDRESS_23   0x081e0000      //128KB

#define flash_protect()                 portENTER_CRITICAL()
#define flash_unprotect()               portEXIT_CRITICAL()

//#define PAGE_ERASED                     0xFFFFFFFF              //for stm32, if erase, value is 0xFF
#define PAGE_ERASED                     0x00000000
#define FLASH_VALID                     0xC33CC33C
#define PAGE_VALID                      0x5A5A5A5A            
#define BLOCK_VALID                     0xA5A5A5A5

//max block size
#define MAX_BLOCK_SIZE                  1024
#define BLOCK_HEAD_SIZE                 8
#define BLOCK_CRC_SIZE                  2

#define CALC_BLOCK_SIZE(buf_size)       ((buf_size)+BLOCK_HEAD_SIZE+BLOCK_CRC_SIZE)
#define CALC_BUFFER_SIZE(block_size)    ((block_size)-BLOCK_HEAD_SIZE-BLOCK_CRC_SIZE)
typedef enum
{
    FLASH_OP_SUCCESS = 0,
    FLASH_OP_INVALID_PARAMETER,
    FLASH_OP_NULL_ERR,
    FLASH_OP_BLOCK_ERR,
    FLASH_OP_CRC_ERR,
    FLASH_OP_SIZE_ERR,
    FLASH_OP_ERASE_ERR,
    FLASH_OP_READ_ERR,
    FLASH_OP_WRITE_ERR,
    FLASH_OP_WRITE_DATA_ERR
}FlashOperatorStatus_t;

typedef enum
{
	FLASH_OP_ACTION_WRITE = 0,
	FLASH_OP_ACTION_READ,
}PageOperationAction_t;

typedef enum
{
    FLASH_SEARCH_OK = 0,
    FLASH_SEARCH_PAGE_EMPTY,
    FLASH_SEARCH_PAGE_MULTI_INVALID,        //only one page valid per time.   
    FLASH_SEARCH_BLOCK_INVALID,
    FLASH_SEARCH_BLOCK_WRITE_FULL_0,
    FLASH_SEARCH_BLOCK_WRITE_FULL_1,
}FlashSearchStatus_t;

typedef struct 
{
    uint32_t valid;
    uint32_t page_addr;
    uint32_t page_size;
    uint32_t block_size;
    uint32_t block_num;
    uint16_t crc;
}PageHeader_t;

//block 
typedef struct
{
    uint32_t valid;
    uint32_t size;      //size of buffer region, not real data size
    uint8_t buffer[];
    //uint32_t crc      //at the end of block, address "size+BLOCK_HEAD_SIZE", crc all region exclude crc 2 byte.
}BlockHeader_t;

typedef struct
{
    uint32_t flash_valid;
    
    uint32_t page0_address;     
    uint32_t page1_address;
    
    uint32_t page_size;         //size for one page
    uint32_t block_size;        //size for one block
    uint32_t block_num;         //support block num, page_size/block_size-1(foir page header)
}FlashInfo_t;

void flash_test_example(void);
BaseType_t flash_check_empty(FlashInfo_t *Handler);
BaseType_t flash_set_empty(FlashInfo_t *Handler);
BaseType_t flash_driver_init(FlashInfo_t *Handler, uint32_t sector_addr0, uint32_t sector_addr1, 
                          uint32_t page_size, uint32_t block_size);
FlashOperatorStatus_t flash_write(FlashInfo_t *Handler, uint8_t *pbuffer, uint16_t size);
FlashOperatorStatus_t flash_read(FlashInfo_t *Handler, uint8_t *pbuffer, uint16_t size);

#endif