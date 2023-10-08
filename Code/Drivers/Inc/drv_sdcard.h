//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      sdmmc.hpp
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
#ifndef _SDCARD_H
#define _SDCARD_H

#include "interface.h"

#define SDMMC_READ_WRITE_TIMEOUT        1000
#define SDMMC_BLOCK_SIZE                512
#define SDMMC_CLOCK_DIV                 2


#ifdef __cplusplus
    extern "C" {
#endif
        
BaseType_t sdcard_driver_init(void);
HAL_StatusTypeDef sdcard_read_disk(uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks);
HAL_StatusTypeDef sdcard_write_disk(const uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks);

#ifdef __cplusplus
    }
#endif
#endif

