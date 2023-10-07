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

#define SPI_USE_HARDWARE_POLL   0
#define SPI_USE_HARDWARE_DMA    1        
#define SPI_USE_SOFTWARE        2

#define SPI_RUN_MODE            SPI_USE_SOFTWARE   

//spi interface
BaseType_t spi_driver_init(void);
uint8_t spi_rw_byte(uint8_t data);
        
uint8_t spi_write_check_ok(void);
uint8_t spi_write_dma(uint8_t *data, uint16_t size);
uint8_t spi_read_check_ok(void);
uint8_t spi_read_dma(uint8_t *data, uint16_t size);        
#ifdef __cplusplus
    }
#endif
    
#endif
