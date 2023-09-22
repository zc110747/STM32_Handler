//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     drv_i2c_mpu9250.h
//
//  Purpose:
//     mpu9250 i2c driver.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DRV_I2C_MPU9250_H
#define _DRV_I2C_MPU9250_H

#include "interface.h"

#define MPU9250_ADDR    	    0x68

#define MPU9250_TIMEOUT         10

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t mpu9250_driver_init(void);
BaseType_t mpu9250_i2c_read_reg(uint8_t reg, uint8_t data);
BaseType_t mpu9250_i2c_multi_read(uint8_t reg, uint8_t *rdata, uint8_t size);    
BaseType_t mpu9250_i2c_write_reg(uint8_t reg, uint8_t data);
BaseType_t mpu9250_i2c_multi_write(uint8_t reg, uint8_t *data, uint8_t size);    
#ifdef __cplusplus
}
#endif

#endif