//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      driver_timer.hpp
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
#ifndef _DRIVER_TIMER_H
#define _DRIVER_TIMER_H

#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t pwm_driver_init(void);
void pwm_set_percent(float percent);

#ifdef __cplusplus
}
#endif

#endif