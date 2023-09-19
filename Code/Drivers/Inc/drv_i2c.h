//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     drv_i2c.h
//
//  Purpose:
//     i2c driver.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DRV_I2C_H
#define _DRV_I2C_H

#include "interface.h"

#define I2C_USE_HARDWARE    0
#define I2C_USE_SOFTWARE    1

#define I2C_RUN_MODE        I2C_USE_SOFTWARE   

#define PCF8574_ADDR 	        0x40
#define PCF8574_I2C_TIMEOUT     100

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t i2c_driver_init(void);
BaseType_t i2c_write(uint8_t addr, uint8_t data);
BaseType_t i2c_read(uint8_t addr,uint8_t *pdata);

#ifdef __cplusplus
}
#endif

#endif