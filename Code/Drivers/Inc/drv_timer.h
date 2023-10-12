//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     drv_timer.h
//
//  Purpose:
//     timer driver.
//
// Author:
//     @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __DRV_TIMER_H
#define __DRV_TIMER_H

#include "interface.h"

#define TIME_EXTEND_ENCODE  0

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t timer_extend_driver_init(void);
uint32_t get_cnt_per_second(void);
#ifdef __cplusplus
}
#endif

#endif
