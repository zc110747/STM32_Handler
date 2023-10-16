//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_sdio.h
//
//  Purpose:
//      sdcard driver interface for init, read, write.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DRV_SDIO_H
#define _DRV_SDIO_H

#include "interface.h"

#define SDIO_MODE_PULL                  0
#define SDIO_MODE_DMA                   1
#define SDIO_RUN_MODE                   SDIO_MODE_DMA

#define SDMMC_READ_WRITE_TIMEOUT        10000
#define SDMMC_BLOCK_SIZE                512
#define SDMMC_CLOCK_DIV                 2

#ifdef __cplusplus
    extern "C" {
#endif
        
BaseType_t sdcard_driver_init(void);
BaseType_t sdcard_driver_test(void);
HAL_StatusTypeDef sdcard_read_disk(uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks);
HAL_StatusTypeDef sdcard_write_disk(const uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks);

#ifdef __cplusplus
    }
#endif
#endif

