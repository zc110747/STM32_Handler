//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_spi_w25q.c
//
//  Purpose:
//      w25qxx chip drive.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_spi.h"
#include "drv_spi_w25q.h"

//local parameter
static uint32_t w25q_chip_id = 0;

//local function
static void wq_write_enable(void);
static void wq_write_disable(void);
static uint16_t wq_read_chipid(void);
static WQ_OP_STATUS wq_wait_busy(uint8_t mode);
static WQ_OP_STATUS wq_enter_4byte_addr(void);

void wq25_read_test(void)
{
    uint8_t buffer[25] = {0};
    
    memcpy(buffer, "hello spi test!", strlen("hello spi test!"));
    
    //if(wq_sector_erase(0) == WQ_OP_OK)
    {
        //wq_memory_write(4096, buffer, 25);
    }
    
    memset(buffer, 0, 25);
    wq_memory_read(0, buffer, 24);
    
    PRINT_LOG(LOG_INFO, "spi wq read info:%s!", buffer);
}

//wq interface
BaseType_t wq25_driver_init(void)
{
    BaseType_t xReturn;
    
    //spi
    xReturn = spi_driver_init();
    if(xReturn == pdPASS)
    {
        w25q_chip_id = wq_read_chipid();
        if(WQ25_GetManufacturerID(w25q_chip_id) != WQ25_ManufacturerID)
        {
            w25q_chip_id = 0;
            xReturn = pdFAIL;
        }
        else
        {
        
            if(w25q_chip_id == CHIP_ID_Q256)        //SPI FLASHΪW25Q256
            {
                wq_enter_4byte_addr();
            }
    
            wq25_read_test();
            
            PRINT_LOG(LOG_INFO, "spi wq init success, id:0x%x!", w25q_chip_id);
        }
    }
    else
    {
        PRINT_LOG(LOG_ERROR, "spi wq init failed!"); 
    }
    return xReturn;
}

static WQ_OP_STATUS wq_enter_4byte_addr(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    //driver must match the manufacturer id
    if(WQ25_GetManufacturerID(w25q_chip_id) != WQ25_ManufacturerID)
        return WQ_OP_DEVICE_ERR;
          
  	WQ25_CS_ON();
    spi_rw_byte(W25X_Enable4ByteAddr, &status);
  	WQ25_CS_OFF();  

    if(status != HAL_OK)
        return WQ_OP_COM_ERR;
    
    return WQ_OP_OK;
}

WQ_OP_STATUS wq_sector_erase(uint32_t addr)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    //driver must match the manufacturer id
    if(WQ25_GetManufacturerID(w25q_chip_id) != WQ25_ManufacturerID)
        return WQ_OP_DEVICE_ERR;
    
    wq_write_enable();           
    if(wq_wait_busy(0) != WQ_OP_OK)
        return WQ_OP_TIMEOUT_ERR;
    
  	WQ25_CS_ON();  

    //erase sector
    spi_rw_byte(W25X_SectorErase, &status); 
    if(status == HAL_OK)
    {
        addr -= addr%W25X_SECTOR_SIZE;
        if(w25q_chip_id == CHIP_ID_Q256)                
        {
            spi_rw_byte((uint8_t)((addr)>>24), NULL); 
        }
        spi_rw_byte((uint8_t)((addr)>>16), NULL);  
        spi_rw_byte((uint8_t)((addr)>>8), NULL);   
        spi_rw_byte((uint8_t)addr, NULL); 
    }
   
    WQ25_CS_OFF();      
    
    if(wq_wait_busy(0) != WQ_OP_OK)
        return WQ_OP_TIMEOUT_ERR; 
    
    if(status != HAL_OK)
       return WQ_OP_COM_ERR; 
    
    return WQ_OP_OK;
}

WQ_OP_STATUS wq_block_erase(uint32_t addr)
{
    HAL_StatusTypeDef status = HAL_OK;
    
     //driver must match the manufacturer id
    if(WQ25_GetManufacturerID(w25q_chip_id) != WQ25_ManufacturerID)
        return WQ_OP_DEVICE_ERR;
    
    wq_write_enable();           
    if(wq_wait_busy(0) != WQ_OP_OK)
        return WQ_OP_TIMEOUT_ERR;
    
  	WQ25_CS_ON();  

    //erase sector
    spi_rw_byte(W25X_BlockErase, &status); 
    if(status == HAL_OK)
    {
        addr -= addr%W25X_BLOCK_SIZE;
        if(w25q_chip_id == CHIP_ID_Q256)                
        {
            spi_rw_byte((uint8_t)((addr)>>24), NULL); 
        }
        spi_rw_byte((uint8_t)((addr)>>16), NULL);  
        spi_rw_byte((uint8_t)((addr)>>8), NULL);   
        spi_rw_byte((uint8_t)addr, NULL); 
    }
    
    WQ25_CS_OFF();      
    
    if(wq_wait_busy(1) != WQ_OP_OK)
        return WQ_OP_TIMEOUT_ERR; 

    if(status != HAL_OK)
       return WQ_OP_COM_ERR; 
      
    return WQ_OP_OK;   
}

WQ_OP_STATUS wq_chip_erase(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    //driver must match the manufacturer id
    if(WQ25_GetManufacturerID(w25q_chip_id) != WQ25_ManufacturerID)
        return WQ_OP_DEVICE_ERR;

    wq_write_enable();           
    if(wq_wait_busy(0) != WQ_OP_OK)
        return WQ_OP_TIMEOUT_ERR; 
    
    WQ25_CS_ON();
    spi_rw_byte(W25X_ChipErase, &status);
    WQ25_CS_OFF();     
    if(wq_wait_busy(1) != WQ_OP_OK)
        return WQ_OP_TIMEOUT_ERR; 
    
    if(status != HAL_OK)
       return WQ_OP_COM_ERR; 
    
    return WQ_OP_OK;      
}

WQ_OP_STATUS wq_memory_read(uint32_t addr, uint8_t *pbuffer, uint16_t num)
{
    uint16_t i;
    HAL_StatusTypeDef status = HAL_OK;
    
    //driver must match the manufacturer id
    if(WQ25_GetManufacturerID(w25q_chip_id) != WQ25_ManufacturerID)
        return WQ_OP_DEVICE_ERR;
    
    WQ25_CS_ON();  
    
    spi_rw_byte(W25X_INSTRU_READ, &status);
    if(status == HAL_OK)
    {
        if(w25q_chip_id == CHIP_ID_Q256)
        {
            spi_rw_byte((uint8_t)(addr>>24), NULL);
        }
        spi_rw_byte((uint8_t)(addr>>16), NULL);
        spi_rw_byte((uint8_t)(addr>>8), NULL);
        spi_rw_byte((uint8_t)(addr), NULL);
        for(i = 0; i<num; i++)
        {
           pbuffer[i] = spi_rw_byte(0xff, NULL);
        }
    }
    
    WQ25_CS_OFF();
    if(status != HAL_OK)
       return WQ_OP_COM_ERR; 
    
    return WQ_OP_OK;    
}

WQ_OP_STATUS wq_memory_write(uint32_t addr, uint8_t *pbuffer, uint16_t num)
{
    uint16_t i;
    HAL_StatusTypeDef status = HAL_OK;
    
    //driver must match the manufacturer id
    if(WQ25_GetManufacturerID(w25q_chip_id) != WQ25_ManufacturerID)
        return pdFAIL;
    
    if(wq_wait_busy(0) != WQ_OP_OK)
        return WQ_OP_TIMEOUT_ERR;
    
    wq_write_enable();
    
    WQ25_CS_ON();
    spi_rw_byte(W25X_PageProgram, &status); 
    
    if(status == HAL_OK)
    {
        if(w25q_chip_id == CHIP_ID_Q256)          
        {
            spi_rw_byte((uint8_t)((addr)>>24), NULL); 
        }
        spi_rw_byte((uint8_t)((addr)>>16), NULL); 
        spi_rw_byte((uint8_t)((addr)>>8), NULL);   
        spi_rw_byte((uint8_t)addr, NULL);   
        for(i=0; i<num; i++)
        {
            spi_rw_byte(pbuffer[i], NULL);
        }
    }
    
    WQ25_CS_OFF();
    
    if(wq_wait_busy(0) != WQ_OP_OK)
        return WQ_OP_TIMEOUT_ERR; 
    
    if(status != HAL_OK)
       return WQ_OP_COM_ERR; 
        
    return WQ_OP_OK;
}

#if SPI_RUN_MODE == SPI_USE_HARDWARE_DMA
WQ_OP_STATUS wq_memory_read_dma(uint32_t addr, uint8_t *pbuffer, uint16_t num)
{
    uint16_t i;
    uint16_t error_count = 0;
    HAL_StatusTypeDef status = HAL_OK;
    
    //driver must match the manufacturer id
    if(WQ25_GetManufacturerID(w25q_chip_id) != WQ25_ManufacturerID)
        return WQ_OP_DEVICE_ERR;
    
    WQ25_CS_ON();  
    
    spi_rw_byte(W25X_INSTRU_READ, &status);
    
    if(status == HAL_OK)
    {
        if(w25q_chip_id == CHIP_ID_Q256)
        {
            spi_rw_byte((uint8_t)(addr>>24), NULL);
        }
        spi_rw_byte((uint8_t)(addr>>16), NULL);
        spi_rw_byte((uint8_t)(addr>>8), NULL);
        spi_rw_byte((uint8_t)(addr), NULL);
      
        spi_read_dma(pbuffer, num);
        do
        {
            if(spi_read_check_ok() == HAL_OK)
            {
                break;
            }
            else
            {
                error_count++;
                if(error_count > W25X_WATI_TIMEOUT) //wait spi send finished
                {
                    return WQ_OP_TIMEOUT_ERR;
                }    
                delay_ms(1);  //when os run, will release the protocol
            }
        }while(1);
    }
    
    WQ25_CS_OFF();
    
    if(status != HAL_OK)
        return WQ_OP_COM_ERR;
    
    return WQ_OP_OK;    
}

WQ_OP_STATUS wq_memory_write_dma(uint32_t addr, uint8_t *pbuffer, uint16_t num)
{
    uint16_t i;
    uint16_t error_count = 0;
    HAL_StatusTypeDef status = HAL_OK;
    
    //driver must match the manufacturer id
    if(WQ25_GetManufacturerID(w25q_chip_id) != WQ25_ManufacturerID)
        return pdFAIL;
    
    if(wq_wait_busy(0) != WQ_OP_OK)
        return WQ_OP_TIMEOUT_ERR;
    
    wq_write_enable();
    
    WQ25_CS_ON();
    spi_rw_byte(W25X_PageProgram, &status); 
    
    if(status == HAL_OK)
    {
        if(w25q_chip_id == CHIP_ID_Q256)          
        {
            spi_rw_byte((uint8_t)((addr)>>24), NULL); 
        }
        spi_rw_byte((uint8_t)((addr)>>16), NULL); 
        spi_rw_byte((uint8_t)((addr)>>8), NULL);   
        spi_rw_byte((uint8_t)addr, NULL);   
        
        spi_write_dma(pbuffer, num);
        do
        {
            if(spi_write_check_ok() == HAL_OK)
            {
                break;
            }
            else
            {
                error_count++;
                if(error_count > W25X_WATI_TIMEOUT) //wait spi send finished
                {
                    return WQ_OP_TIMEOUT_ERR;
                }    
                delay_ms(1);  //when os run, will release the protocol
            }
        }while(1);    
    }
    WQ25_CS_OFF();
    
    if(wq_wait_busy(0) != WQ_OP_OK)
        return WQ_OP_TIMEOUT_ERR; 
    
    if(status != HAL_OK)
       return WQ_OP_COM_ERR; 
    
    return WQ_OP_OK;
}
#endif

static WQ_OP_STATUS wq_wait_busy(uint8_t mode)
{
    uint32_t error_count = 0, timeout;
    uint8_t reg_status;
    WQ_OP_STATUS status = WQ_OP_OK;
    
    if(mode == 0)
        timeout = W25X_WATI_TIMEOUT;
    else
        timeout = W25X_WATI_LONG_TIMEOUT;    
    
    do
    {
        //read status regs, the last byte is busy
        WQ25_CS_ON();
        spi_rw_byte(W25X_INSTRU_RDSR_1, NULL);
        reg_status = spi_rw_byte(0xff, NULL);
        WQ25_CS_OFF();
        
        if((reg_status&0x1) == 0)
        {
            break;
        }
        else
        {
            error_count++;
            if(error_count>= timeout)
            {
               status = WQ_OP_TIMEOUT_ERR;
               break;
            }
            delay_ms(1);
        }                 
    }while(1);
    
    return status;
}


static void wq_write_enable(void)
{
    WQ25_CS_ON();
    spi_rw_byte(W25X_INSTRU_WREN, NULL);
    WQ25_CS_OFF(); 
}

static void wq_write_disable(void)
{
    WQ25_CS_ON();
    spi_rw_byte(W25X_INSTRU_WRDI, NULL);
    WQ25_CS_OFF();    
}

static uint16_t wq_read_chipid(void)
{
    uint16_t id = 0;
    
    WQ25_CS_ON();
    spi_rw_byte(0x90, NULL);
    spi_rw_byte(0x00, NULL);
    spi_rw_byte(0x00, NULL);
    spi_rw_byte(0x00, NULL);
    id = spi_rw_byte(0xff, NULL)<<8;
    id |= spi_rw_byte(0xff, NULL);
    WQ25_CS_OFF(); 
    
    return id;  
}

