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
#ifndef _DRV_SPI_W25Q_H
#define _DRV_SPI_W25Q_H

#ifdef __cplusplus
    extern "C" {
#endif        

#include "main.h"

#define WQ25_CS_ON()                HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
#define WQ25_CS_OFF()               HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);

#define WQ25_ManufacturerID         0xEF
#define WQ25_GetManufacturerID(id)  (((id)>>8)&0xff)
        
//w25qxx device instruct
#define W25X_INSTRU_WREN		    0x06 
#define W25X_INSTRU_WRDI		    0x04 
#define W25X_INSTRU_RDSR_1		    0x05 
#define W25X_INSTRU_READ			0x03 
#define W25X_PageProgram		    0x02 
#define W25X_BlockErase			    0xD8 
#define W25X_SectorErase		    0x20 
#define W25X_ChipErase			    0xC7 
#define W25X_PowerDown			    0xB9 
#define W25X_ReleasePowerDown	    0xAB 
#define W25X_DeviceID			    0xAB 
#define W25X_ManufactDeviceID	    0x90 
#define W25X_JedecDeviceID		    0x9F 
#define W25X_Enable4ByteAddr        0xB7
#define W25X_Exit4ByteAddr          0xE9   

//chip id
#define CHIP_ID_Q8                  0xEF13
#define CHIP_ID_Q16                 0xEF14
#define CHIP_ID_Q32                 0xEF15
#define CHIP_ID_Q64                 0xEF16
#define CHIP_ID_Q128                0xEF17
#define CHIP_ID_Q256                0xEF18

#define W25X_WATI_TIMEOUT           200
#define W25X_WATI_LONG_TIMEOUT      5000

#define W25X_SECTOR_SIZE            (4*1024)
#define W25X_BLOCK_SIZE             (64*1024)

typedef enum
{
    WQ_OP_OK = 0,
    WQ_OP_DEVICE_ERR,
    WQ_OP_TIMEOUT_ERR,
}WQ_OP_STATUS;

//wq interface
BaseType_t wq25_driver_init(void);

WQ_OP_STATUS wq_chip_erase(void);
WQ_OP_STATUS wq_sector_erase(uint32_t sector);
WQ_OP_STATUS wq_block_erase(uint32_t sector);

WQ_OP_STATUS wq_memory_read(uint32_t addr, uint8_t *pbuffer, uint16_t num);
WQ_OP_STATUS wq_memory_write(uint32_t addr, uint8_t *pbuffer, uint16_t num);    

#ifdef __cplusplus
}
#endif
#endif