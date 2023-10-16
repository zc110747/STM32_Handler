//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      rtc.hpp
//
//  Purpose:
//      rtc interface driver.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __DRV_RTC_H
#define __DRV_RTC_H

#include "interface.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
}RTC_INFO;

BaseType_t rtc_driver_init(void);

//rtc info process
RTC_INFO rtc_update(void); 
void rtc_time_read(RTC_INFO *pGetInfo);

//rtc alarm process
BaseType_t rtc_alarm_state_read(void);   
void rtc_alarm_state_write(BaseType_t type);        
void rtc_alarm_register(uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
        
#ifdef __cplusplus
}
#endif
#endif
