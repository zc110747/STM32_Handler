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
_Pragma("once")

#include "interface.h"

//reference voltage, uint:mv
#define DAC_REFERENCE_VOL   3300

//dac max output value
#define DAC_MAX_VALUE       4096

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t dac_init();    
void dac_set_voltage(uint16_t mv);
    
#ifdef __cplusplus
}
#endif   