//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      dac.h
//
//  Purpose:
//      dac interface init and set_voltage.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __DRV_DAC_H
#define __DRV_DAC_H

#include "interface.h"

#define DMA_MODE_POLL               0
#define DMA_MODE_DMA                1

#define DMA_RUN_MODE                DMA_MODE_DMA

//reference voltage, uint:mv
#define DAC_REFERENCE_VOL           3300

//dac max output value
#define DAC_MAX_VALUE               4095

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t dac_init(void);    
void dac_set_voltage(uint16_t mv);
void set_convert_vol(float percent);
    
#ifdef __cplusplus
}
#endif   
#endif
