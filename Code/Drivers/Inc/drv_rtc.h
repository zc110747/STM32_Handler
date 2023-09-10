//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     rtc.hpp
//
//  Purpose:
//     rtc interface driver.
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

#ifdef __cplusplus
    extern "C" {
#endif

BaseType_t rtc_driver_init(void);
BaseType_t rtc_update(void);
        
RTC_TimeTypeDef *rtc_get_time(void);
RTC_DateTypeDef *rtc_get_date(void);       
BaseType_t rtc_get_alarm_flag(void);   
void rtc_set_alarm_flag(BaseType_t type);        
void rtc_delay_alarm(uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
        
 #ifdef __cplusplus
    }
#endif
    