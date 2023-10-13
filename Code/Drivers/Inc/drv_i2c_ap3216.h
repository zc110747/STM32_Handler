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
#ifndef _DRV_I2C_AP3216_H
#define _DRV_I2C_AP3216_H

#include "interface.h"

#define AP3216C_ADDR    	    0X1E	

/* AP3316C�Ĵ��� */
#define AP3216C_SYSTEMCONG	    0x00	
#define AP3216C_INTSTATUS	    0X01	
#define AP3216C_INTCLEAR	    0X02	
#define AP3216C_IRDATALOW	    0x0A	
#define AP3216C_IRDATAHIGH	    0x0B
#define AP3216C_ALSDATALOW	    0x0C	
#define AP3216C_ALSDATAHIGH	    0X0D	
#define AP3216C_PSDATALOW	    0X0E	
#define AP3216C_PSDATAHIGH	    0X0F	

#define AP3216C_TIMEOUT         10

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t ap3216_driver_init(void);
BaseType_t ap3216_i2c_read_reg(uint8_t reg, uint8_t data);
BaseType_t ap3216_i2c_multi_read(uint8_t reg, uint8_t *rdata, uint8_t size);    
BaseType_t ap3216_i2c_write_reg(uint8_t reg, uint8_t data);
BaseType_t ap3216_i2c_multi_write(uint8_t reg, uint8_t *data, uint8_t size);    
#ifdef __cplusplus
}
#endif

#endif