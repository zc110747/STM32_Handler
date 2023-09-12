//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     rtc.cpp
//
//  Purpose:
//     rtc driver.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_rtc.h"

#define RTC_DATE_YEAR       23
#define RTC_DATE_MONTH      9
#define RTC_DATE_DAY        4
#define RTC_DATE_WEEK       1

#define RTC_TIME_HOUR       21
#define RTC_TIME_MINUTE     6
#define RTC_TIME_SECOND     0

#define RTC_SET_FLAGS       0x5B5B
#define RTC_FORMAT_MODE     RTC_FORMAT_BIN

static RTC_INFO rtc_info_;
static RTC_HandleTypeDef rtc_handler_;
static RTC_AlarmTypeDef rtc_arm_handler_;
static BaseType_t is_alarm = pdFALSE;

static BaseType_t rtc_hardware_init(void);
static HAL_StatusTypeDef rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec);
static HAL_StatusTypeDef rtc_set_date(uint8_t year, uint8_t month, uint8_t date, uint8_t week);

BaseType_t rtc_driver_init(void)
{
    BaseType_t result;

    //unlock backup register update.
    HAL_PWR_EnableBkUpAccess();

    result = rtc_hardware_init();

    if(result == pdPASS)
    {
        rtc_update();
        rtc_delay_alarm(0, 0, 0, 5);
    }
    else
    {
        printf("rtc_driver hardware_init failed\r\n");
    }
    return result;   
}

RTC_INFO rtc_get_info(void)
{
    return rtc_info_;
}

void rtc_set_alarm(uint8_t week, uint8_t hour, uint8_t min, uint8_t sec)
{
    rtc_arm_handler_.AlarmTime.Hours = hour;  
    rtc_arm_handler_.AlarmTime.Minutes = min; 
    rtc_arm_handler_.AlarmTime.Seconds = sec; 
    rtc_arm_handler_.AlarmTime.SubSeconds = 0;
    rtc_arm_handler_.AlarmTime.TimeFormat = RTC_HOURFORMAT_24;
    
    rtc_arm_handler_.AlarmMask = RTC_ALARMMASK_NONE;
    rtc_arm_handler_.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
    rtc_arm_handler_.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
    rtc_arm_handler_.AlarmDateWeekDay = week; 
    rtc_arm_handler_.Alarm = RTC_ALARM_A;     
    
    HAL_RTC_SetAlarm_IT(&rtc_handler_, &rtc_arm_handler_, RTC_FORMAT_MODE);
    
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0x01, 0x02);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

void rtc_delay_alarm(uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
    uint8_t alarm_week = 0;
    uint8_t alarm_hour = 0;
    uint8_t alarm_min = 0;
    uint8_t alarm_sec = 0;

    alarm_sec += rtc_info_.time.Seconds + sec;
    if(alarm_sec >= 60)
    {
        alarm_sec -= 60;
        alarm_min++;
    }
    
    alarm_min += rtc_info_.time.Minutes + min;
    if(alarm_min >= 60)
    {
        alarm_min -= 60;
        alarm_hour++;
    }
    
    alarm_hour += rtc_info_.time.Hours + hour;
    if(alarm_hour >= 24)
    {
        alarm_hour -= 24;
        alarm_week++;
    }
    
    alarm_week += rtc_info_.date.WeekDay + day;
    if(alarm_week > 7)
    {
        alarm_week -= 7;
    }
    
    rtc_set_alarm(alarm_week, alarm_hour, alarm_min, alarm_sec);
}

RTC_INFO rtc_update(void)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    
    HAL_RTC_GetTime(&rtc_handler_, &time, RTC_FORMAT_MODE);
    HAL_RTC_GetDate(&rtc_handler_, &date, RTC_FORMAT_MODE);
    
    //must entery critical to avoid read not matching
    portENTER_CRITICAL();
    rtc_info_.time = time;
    rtc_info_.date = date;
    portEXIT_CRITICAL();
    
    return rtc_info_;
}

void RTC_Alarm_IRQHandler(void)
{
  RTC_HandleTypeDef *prtc_handler = &rtc_handler_;
    
  if(__HAL_RTC_ALARM_GET_IT(prtc_handler, RTC_IT_ALRA))
  {
    if((uint32_t)(prtc_handler->Instance->CR & RTC_IT_ALRA) != (uint32_t)RESET)
    {
      is_alarm = pdTRUE;
        
      /* Clear the Alarm interrupt pending bit */
      __HAL_RTC_ALARM_CLEAR_FLAG(prtc_handler,RTC_FLAG_ALRAF);
    }
  }
  
    /* Clear the EXTI's line Flag for RTC Alarm */
  __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();
  
  /* Change RTC state */
  prtc_handler->State = HAL_RTC_STATE_READY; 
}

void rtc_set_alarm_flag(BaseType_t type)
{
    HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
    is_alarm = type;
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

BaseType_t rtc_get_alarm_flag(void)
{
    return is_alarm;
}    

static BaseType_t rtc_hardware_init(void)
{
    rtc_handler_.Instance = RTC;
    rtc_handler_.Init.HourFormat = RTC_HOURFORMAT_24;
    rtc_handler_.Init.AsynchPrediv = 127;
    rtc_handler_.Init.SynchPrediv = 255;
    rtc_handler_.Init.OutPut = RTC_OUTPUT_DISABLE;
    rtc_handler_.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    rtc_handler_.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&rtc_handler_) != HAL_OK)
        return pdFAIL;

    if(HAL_RTCEx_BKUPRead(&rtc_handler_, RTC_BKP_DR0) != RTC_SET_FLAGS)
    {
        rtc_set_time(RTC_TIME_HOUR, RTC_TIME_MINUTE, RTC_TIME_SECOND);
        rtc_set_date(RTC_DATE_YEAR, RTC_DATE_MONTH, RTC_DATE_DAY, RTC_DATE_WEEK);

        HAL_RTCEx_BKUPWrite(&rtc_handler_, RTC_BKP_DR0, RTC_SET_FLAGS);
    }

    return pdPASS;   
}

static HAL_StatusTypeDef rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec)
{
    rtc_info_.time.Hours = hour;
    rtc_info_.time.Minutes = min;
    rtc_info_.time.Seconds = sec;
    rtc_info_.time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    rtc_info_.time.StoreOperation = RTC_STOREOPERATION_RESET;
    
    return HAL_RTC_SetTime(&rtc_handler_, &rtc_info_.time, RTC_FORMAT_MODE);
}

static HAL_StatusTypeDef rtc_set_date(uint8_t year, uint8_t month, uint8_t date, uint8_t week)
{
    rtc_info_.date.Year = year;
    rtc_info_.date.Month = month;
    rtc_info_.date.Date = date;
    rtc_info_.date.WeekDay = week;
    
    return HAL_RTC_SetDate(&rtc_handler_, &rtc_info_.date, RTC_FORMAT_MODE);
}
