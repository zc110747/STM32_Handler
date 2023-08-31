//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     i2c.cpp
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
_Pragma("once")

#include "interface.h"

#define PCF8574_ADDR 	        0x40
#define PCF8574_I2C_TIMEOUT     100

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t i2c_init(void);
BaseType_t i2c_write(uint8_t addr, uint8_t data);
BaseType_t i2c_read(uint8_t addr,uint8_t *pdata);

#ifdef __cplusplus
}
#endif