//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_adc.hpp
//
//  Purpose:
//      adc driver normal get.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DRV_ADC_H
#define _DRV_ADC_H

#include "interface.h"

#define ADC_RUN_NORMAL              0
#define ADC_RUN_DMA                 1

#define ADC_RUN_MODE                ADC_RUN_DMA

#define ADC_AVG_TIMES               5

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t adc_driver_init(void);
uint16_t adc_get_avg(uint32_t channel);
    
#ifdef __cplusplus
}
#endif

#endif
