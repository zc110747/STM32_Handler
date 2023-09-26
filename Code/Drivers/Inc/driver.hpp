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

#include "drv_usart.h"
#include "drv_rtc.h"
#include "drv_led.h"
#include "drv_pwm.h"
#include "drv_tpad.h"
#include "drv_dac.h"
#include "drv_alg.h"
#include "drv_wdg.h"
#include "drv_adc.h"
#include "drv_spi.h"
#include "drv_dma.h"
#include "drv_key.h"
#include "drv_sdcard.h"
#include "drv_sdram.h"

#include "drv_i2c_pcf8574.h"
#include "drv_i2c_ap3216.h"

#include "SEGGER_RTT.h"

#ifdef __cplusplus
	extern "C" {
#endif

BaseType_t driver_init(void);
        
#ifdef __cplusplus
	}
#endif
#endif
