//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      adc.hpp
//
//  Purpose:
//     adc driver normal get.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "interface.h"

#define ADC_AVG_TIMES   5

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t adc_init(void);
uint16_t adc_get_value(uint32_t channel);
uint16_t adc_get_avg(uint32_t channel);
    
#ifdef __cplusplus
}
#endif