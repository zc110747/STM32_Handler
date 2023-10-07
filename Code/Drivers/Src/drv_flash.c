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
#include "drv_flash.h"
#include "logger_process.h"

static uint32_t block_buffer[MAX_BLOCK_SIZE/4+1];

//local function
static FlashSearchStatus_t flash_find_block(FlashInfo_t *Handler, PageOperationAction_t Operation,
                                            uint32_t *find_addr);
static uint16_t flash_calc_crc16(uint8_t *ptr, uint16_t len);

#ifdef STM32F429xE
__weak uint8_t flash_get_sector(uint32_t address)
{
    if(address<FLASH_SECTOR_START_ADDRESS_1)        return FLASH_SECTOR_0;
    else if(address<FLASH_SECTOR_START_ADDRESS_2)   return FLASH_SECTOR_1;
    else if(address<FLASH_SECTOR_START_ADDRESS_3)   return FLASH_SECTOR_2;
    else if(address<FLASH_SECTOR_START_ADDRESS_4)   return FLASH_SECTOR_3;
    else if(address<FLASH_SECTOR_START_ADDRESS_5)   return FLASH_SECTOR_4;
    else if(address<FLASH_SECTOR_START_ADDRESS_6)   return FLASH_SECTOR_5;
    else if(address<FLASH_SECTOR_START_ADDRESS_7)   return FLASH_SECTOR_6;
    else if(address<FLASH_SECTOR_START_ADDRESS_8)   return FLASH_SECTOR_7;
    else if(address<FLASH_SECTOR_START_ADDRESS_9)   return FLASH_SECTOR_8;
    else if(address<FLASH_SECTOR_START_ADDRESS_10)  return FLASH_SECTOR_9;
    else if(address<FLASH_SECTOR_START_ADDRESS_11)  return FLASH_SECTOR_10;
    else if(address<FLASH_SECTOR_START_ADDRESS_12)  return FLASH_SECTOR_11;
    else if(address<FLASH_SECTOR_START_ADDRESS_13)  return FLASH_SECTOR_12;
    else if(address<FLASH_SECTOR_START_ADDRESS_14)  return FLASH_SECTOR_13;
    else if(address<FLASH_SECTOR_START_ADDRESS_15)  return FLASH_SECTOR_14;
    else if(address<FLASH_SECTOR_START_ADDRESS_16)  return FLASH_SECTOR_15;
    else if(address<FLASH_SECTOR_START_ADDRESS_17)  return FLASH_SECTOR_16;
    else if(address<FLASH_SECTOR_START_ADDRESS_18)  return FLASH_SECTOR_17;
    else if(address<FLASH_SECTOR_START_ADDRESS_19)  return FLASH_SECTOR_18;
    else if(address<FLASH_SECTOR_START_ADDRESS_20)  return FLASH_SECTOR_19;
    else if(address<FLASH_SECTOR_START_ADDRESS_21)  return FLASH_SECTOR_20;
    else if(address<FLASH_SECTOR_START_ADDRESS_22)  return FLASH_SECTOR_21;
    else if(address<FLASH_SECTOR_START_ADDRESS_23)  return FLASH_SECTOR_22;
    else                                            return FLASH_SECTOR_23;
}

__weak BaseType_t flash_empty_device(uint32_t address, uint32_t size)
{
    BaseType_t xReturn = pdPASS;
    FLASH_EraseInitTypeDef FlashEraseInit = {0};
    uint32_t sector_error;
    
    HAL_FLASH_Unlock(); 

    //erase page0
    FlashEraseInit.NbSectors = 1;
    FlashEraseInit.Sector = flash_get_sector(address);
    FlashEraseInit.Banks = FLASH_BANK_1;
    FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    if(HAL_FLASHEx_Erase(&FlashEraseInit, &sector_error) != HAL_OK)  
    {
        HAL_FLASH_Lock(); 
        return pdFAIL;
    }

    HAL_FLASH_Lock(); 
    return pdPASS;
}

__weak BaseType_t flash_write_device(uint32_t address, uint32_t *pbuffer, uint32_t size)
{
    uint16_t index;
    
    HAL_FLASH_Unlock();       
    for(index=0; index<size; index++)
    {
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address+index*4, pbuffer[index]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return pdFAIL;
        }
    } 
    HAL_FLASH_Lock();
    
    FLASH_FlushCaches(); 
    return pdPASS;
}
#else
__weak uint8_t flash_get_sector(uint32_t address)
{
    return address;
}

__weak BaseType_t flash_empty_device(uint32_t address, uint32_t size)
{
    memset((char *)address, 0, size);
    return pdPASS;
}

__weak BaseType_t flash_write_device(uint32_t address, uint32_t *pbuffer, uint32_t size)
{
    uint16_t index;
    
    for(index=0; index<size; index++) 
    {
        *((uint32_t *)(address+index*4)) = pbuffer[index];
    }
    return pdPASS;
}

__weak BaseType_t flash_read_device(uint32_t address, uint32_t *pbuffer, uint32_t size)
{
    uint16_t index;
    
    for(index=0; index<size; index++) 
    {
        pbuffer[index] = *((uint32_t *)(address+index*4));
    }
    return pdPASS;    
}
#endif

//1.sector_addr0/sector_addr1 must in different sector, start of the sector is best.
//2.all the device need div by 4 for flash write need.
//3.to simplify the two buffer with same page_size and block_size
BaseType_t flash_driver_init(FlashInfo_t *Handler, uint32_t sector_addr0, uint32_t sector_addr1, 
                          uint32_t page_size, uint32_t block_size)
{
    if(Handler == NULL)
        return pdFAIL;
    
    if(page_size == 0 || block_size == 0 
       || block_size > MAX_BLOCK_SIZE || page_size<(block_size*2))
        return pdFAIL;
    
    if(((sector_addr0 | sector_addr1 | page_size | block_size)&0x4) != 0)
        return pdFAIL;
    
    Handler->page0_address = sector_addr0;
    Handler->page1_address = sector_addr1;
    Handler->page_size = page_size;
    Handler->block_size = block_size;
    Handler->block_num = page_size/block_size-1; //first block used for page info
    Handler->flash_valid = FLASH_VALID;
    
    return pdPASS;
}

FlashOperatorStatus_t flash_read(FlashInfo_t *Handler, uint8_t *pbuffer, uint16_t size)
{
    FlashSearchStatus_t search_status;
    uint32_t block_address;
    uint8_t *pcrc;
    
    if(Handler == NULL || pbuffer == NULL)
        return FLASH_OP_INVALID_PARAMETER;
   
    search_status = flash_find_block(Handler, FLASH_OP_ACTION_READ, &block_address);
    if(search_status == FLASH_SEARCH_OK)
    {
        uint16_t soft_crc, flash_crc;
       
        if(flash_read_device(block_address, block_buffer, Handler->block_size/4) == pdPASS)
        {           
            BlockHeader_t *pHeader = (BlockHeader_t *)block_buffer;
            if(CALC_BLOCK_SIZE(pHeader->size) != Handler->block_size)
            {
                return FLASH_OP_SIZE_ERR;
            }
            
            soft_crc = flash_calc_crc16((uint8_t *)pHeader, Handler->block_size-2);
            pcrc = (uint8_t *)(block_address+Handler->block_size-2);
            flash_crc = pcrc[0]<<8 | pcrc[1];
            if(soft_crc == flash_crc)
            {
                //for flash, just copy the data from flash to buffer
                memcpy((char *)pbuffer, (char *)pHeader->buffer, size);
                return FLASH_OP_SUCCESS;
            }
            else
            {
                return FLASH_OP_CRC_ERR;
            }
        }
        else
        {
            return FLASH_OP_READ_ERR;
        }
    }
    
    return FLASH_OP_BLOCK_ERR;
}

BaseType_t flash_check_empty(FlashInfo_t *Handler)
{
    if((((PageHeader_t *)Handler->page0_address)->valid == PAGE_ERASED)
    &&(((PageHeader_t *)Handler->page1_address)->valid  == PAGE_ERASED))
        return pdTRUE;
    
    return pdFALSE;
}

FlashOperatorStatus_t flash_write(FlashInfo_t *Handler, uint8_t *pbuffer, uint16_t size)
{
    FlashSearchStatus_t search_status;
    uint32_t block_address;
    uint16_t soft_crc;
    uint8_t *pcrc;
    BlockHeader_t *pBlockHeader;
    PageHeader_t *pPageHeader;
    
    if(Handler == NULL || pbuffer == NULL)
        return FLASH_OP_INVALID_PARAMETER;

    //1.search valid block to write
    search_status = flash_find_block(Handler, FLASH_OP_ACTION_WRITE, &block_address);
    
    //2.if search failed, process page valid flag.
    if(search_status != FLASH_SEARCH_OK)
    {
        memset((char *)block_buffer, 0, MAX_BLOCK_SIZE);
        pPageHeader = (PageHeader_t *)block_buffer;
  
        pPageHeader->valid = PAGE_VALID;
        pPageHeader->page_size = Handler->page_size;
        pPageHeader->block_num = Handler->block_num;
        pPageHeader->block_size = Handler->block_size;
        
        if(search_status == FLASH_SEARCH_PAGE_EMPTY
        ||  search_status == FLASH_SEARCH_PAGE_MULTI_INVALID)
        {
            if(((PageHeader_t *)(Handler->page0_address))->valid != PAGE_ERASED)
            {
                flash_empty_device(Handler->page0_address, Handler->page_size);
            }
            
            if(((PageHeader_t *)(Handler->page1_address))->valid != PAGE_ERASED)
            {
                flash_empty_device(Handler->page1_address, Handler->page_size);
            } 
            
            pPageHeader->page_addr = Handler->page0_address;
        }
        else if(search_status == FLASH_SEARCH_BLOCK_WRITE_FULL_0)
        {
            if(((PageHeader_t *)(Handler->page1_address))->valid != PAGE_ERASED)
            {
                flash_empty_device(Handler->page1_address, Handler->page_size);
            } 
            pPageHeader->page_addr = Handler->page1_address;  
        }
        else if(search_status == FLASH_SEARCH_BLOCK_WRITE_FULL_1)
        {
            if(((PageHeader_t *)(Handler->page0_address))->valid != PAGE_ERASED)
            {
                flash_empty_device(Handler->page0_address, Handler->page_size);
            } 
            pPageHeader->page_addr = Handler->page0_address;  
        }
        pPageHeader->crc = flash_calc_crc16((uint8_t *)block_buffer, sizeof(PageHeader_t)-2);
        
        //write header
        if(flash_write_device(pPageHeader->page_addr, block_buffer, Handler->block_size/4) != pdPASS)
            return FLASH_OP_WRITE_ERR;  

        block_address = pPageHeader->page_addr + Handler->block_size;
    }

    //2.process block
    pBlockHeader = (BlockHeader_t *)block_buffer;
    memset((char *)pBlockHeader, 0, MAX_BLOCK_SIZE); 
    pBlockHeader->valid = BLOCK_VALID;
    pBlockHeader->size = CALC_BUFFER_SIZE(Handler->block_size);
    memcpy((char *)pBlockHeader->buffer, pbuffer, size);
    soft_crc = flash_calc_crc16((uint8_t *)block_buffer, Handler->block_size-2);
    pcrc = (uint8_t *)(((uint32_t)block_buffer)+Handler->block_size-2);
    pcrc[0] = soft_crc>>8;
    pcrc[1] = soft_crc&0xff;
    
    if(flash_write_device(block_address, block_buffer, Handler->block_size/4) != pdPASS)
        return FLASH_OP_WRITE_ERR;
    
    //if write data error, memory damage, as hardware error
    if(memcmp((char *)block_buffer, (char *)block_address, Handler->block_size) != 0)
        return FLASH_OP_WRITE_DATA_ERR;
    
    if(search_status == FLASH_SEARCH_BLOCK_WRITE_FULL_0)
    {
        flash_empty_device(Handler->page0_address, Handler->page_size);
    }
    if(search_status == FLASH_SEARCH_BLOCK_WRITE_FULL_1)
    {
        flash_empty_device(Handler->page1_address, Handler->page_size);
    }
    
    return FLASH_OP_SUCCESS;
}

BaseType_t flash_set_empty(FlashInfo_t *Handler)
{
    if(flash_empty_device(Handler->page0_address, Handler->page_size) != pdPASS)
        return pdFAIL;
    
    if(flash_empty_device(Handler->page1_address, Handler->page_size) != pdPASS)
        return pdFAIL;
     
    return pdPASS;
}

//////////////////////////////////////////////////
static FlashSearchStatus_t flash_find_block(FlashInfo_t *Handler, PageOperationAction_t Operation,
                                            uint32_t *find_addr)
{
    uint32_t page_valid_0, page_valid_1;
    uint32_t page_valid_addr, block_valid_addr;
    uint16_t block_index, block_num, block_size;
     
    page_valid_0 = *(uint32_t *)(Handler->page0_address);
    page_valid_1 = *(uint32_t *)(Handler->page1_address);    
  
    //1.multi valid page is invalid
    if(page_valid_0 == PAGE_VALID && page_valid_1 == PAGE_VALID)
        return FLASH_SEARCH_PAGE_MULTI_INVALID;
  
    //2.find valid page
    if(page_valid_0 == PAGE_VALID)
    {
        page_valid_addr = Handler->page0_address;
    }
    else if(page_valid_1 == PAGE_VALID)
    {
        page_valid_addr = Handler->page1_address; 
    }
    else
    {
        return FLASH_SEARCH_PAGE_EMPTY; 
    }
    
    block_num = Handler->block_num;
    block_size = Handler->block_size;
    
    switch(Operation)
    {
        case FLASH_OP_ACTION_WRITE:
            for(block_index=0; block_index<block_num; block_index++)
            {
                block_valid_addr = page_valid_addr + (block_index+1)*block_size;
                if(((BlockHeader_t *)block_valid_addr)->valid == PAGE_ERASED)
                {
                    *find_addr = block_valid_addr;
                    return FLASH_SEARCH_OK;
                }
            }
            
            if(page_valid_addr == Handler->page0_address)
            {
                return FLASH_SEARCH_BLOCK_WRITE_FULL_0;
            }
            else if(page_valid_addr == Handler->page1_address)
            {
                return FLASH_SEARCH_BLOCK_WRITE_FULL_1;
            }
            break;
        case FLASH_OP_ACTION_READ:
            for(block_index=0; block_index<block_num; block_index++)
            {
                //structure page_valid region + block_num*block_size
                //block valid addr region is [page_valid_addr+block_size, page_valid_addr+block_size*block_num]
                block_valid_addr = page_valid_addr + (block_num - block_index)*block_size;
                if(((BlockHeader_t *)block_valid_addr)->valid == BLOCK_VALID)
                {
                    *find_addr = block_valid_addr;
                    return FLASH_SEARCH_OK;
                }
            }
            break;
    }
    
    return FLASH_SEARCH_BLOCK_INVALID;
}

static uint16_t flash_calc_crc16(uint8_t *ptr, uint16_t len)
{
    uint8_t i;
    uint16_t crc = 0xFFFF;
    while(len--)
    {
        crc ^= *ptr++;            
        for (i = 0; i < 8; ++i)
        {
            if (crc & 0x80)
                crc = (crc << 1)^0xA001;        
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

void flash_test_example(void)
{
    #define FLASH_BUFFER_SIZE       100
    FlashInfo_t example_flash0 = {0};
    uint8_t index;
    uint8_t flash_write_list[FLASH_BUFFER_SIZE], flash_read_list[FLASH_BUFFER_SIZE];
    FlashOperatorStatus_t op_status;
    
    for(index = 0; index<100; index++)
    {
        flash_write_list[index] = index;
    }
    
    flash_driver_init(&example_flash0, FLASH_SECTOR_START_ADDRESS_10, FLASH_SECTOR_START_ADDRESS_11, (128*1024), 768);
    
    //check empty, may be first times
    if(flash_check_empty(&example_flash0))
    {
        op_status = flash_write(&example_flash0, flash_write_list, FLASH_BUFFER_SIZE);
        if(op_status != FLASH_OP_SUCCESS)
        {
            PRINT_LOG(LOG_ERROR, "FLASH Write Operator Failed:%d!", op_status);
        }
    }
    
    op_status = flash_read(&example_flash0, flash_read_list, FLASH_BUFFER_SIZE);
    if(op_status == FLASH_OP_SUCCESS)
    {
        if(memcmp((char *)flash_write_list, (char *)flash_read_list, FLASH_BUFFER_SIZE) == 0)
        {
           PRINT_LOG(LOG_ERROR, "FLASH Read read test ok!"); 
        }
        else
        {
           PRINT_LOG(LOG_ERROR, "FLASH Read read test failed!"); 
        }
    }
    else
    {
        PRINT_LOG(LOG_ERROR, "FLASH Read read test falid, status:%d!", op_status);         
    }
}