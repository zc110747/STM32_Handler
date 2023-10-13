//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_i2c.h
//
//  Purpose:
//      i2c driver.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DRV_I2C_PCF8574_H
#define _DRV_I2C_PCF8574_H

#include "interface.h"

#define PCF8574_ADDR 	        0x40
#define PCF8574_I2C_TIMEOUT     10

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t pcf8574_driver_init(void);
BaseType_t pcf8574_i2c_write(uint8_t data);
BaseType_t pcf8574_i2c_read(uint8_t *pdata);

#ifdef __cplusplus
}
#endif

#endif