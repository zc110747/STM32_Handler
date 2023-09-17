//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      driver.h
//
//  Purpose:
//      include all hardware driver interface.
//
// Author:
//      @zc
//
//  Assumptions:
//	
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __DRIVER_H
#define __DRIVER_H

#include "key.h"
#include "spi.h"
#include "dma.h"
#include "dsp_test.h"

#include "drv_usart.h"
#include "drv_rtc.h"
#include "drv_led.h"
#include "drv_pwm.h"
#include "drv_tpad.h"
#include "drv_dac.h"
#include "drv_alg.h"
#include "drv_wdg.h"
#include "drv_adc.h"
#include "drv_i2c.h"

#include "SEGGER_RTT.h"

#ifdef __cplusplus
	extern "C" {
#endif

BaseType_t driver_init(void);		
HAL_StatusTypeDef read_disk(uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks);
HAL_StatusTypeDef write_disk(const uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks);
        
#ifdef __cplusplus
	}
#endif
#endif
