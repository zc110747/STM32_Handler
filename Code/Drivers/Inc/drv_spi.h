//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_spi.h
//
//  Purpose:
//      spi driver for flash.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DRV_SPI_H
#define _DRV_SPI_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "interface.h"

#define SPI_USE_HARDWARE        0
#define SPI_USE_SOFTWARE        1

#define SPI_RUN_MODE            SPI_USE_SOFTWARE   

//spi interface
BaseType_t spi_driver_init(void);
uint8_t spi_rw_byte(uint8_t data);
#ifdef __cplusplus
    }
#endif
    
#endif
