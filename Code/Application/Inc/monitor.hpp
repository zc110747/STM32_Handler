//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      monitor.hpp
//
//  Purpose:
//      system monitor functional.
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

#include "driver.h"
#include "includes.hpp"

#define KEY_NUM     2

class monitor_manage
{
public: 
    /// \brief constructor.  
    monitor_manage() {}

    /// \brief destructor.    
    ~monitor_manage() {}   

    BaseType_t init();
    
    static monitor_manage* get_instance()
    {
        static monitor_manage instance_;
        return &instance_;
    }

    KEY_STATE anti_shake(uint8_t *pTick, KEY_STATE nowIoStatus, KEY_STATE readIoStatus);
    bool is_time_escape(uint32_t ticks, uint32_t time);
    uint32_t get_per_second(){
        return per_second;
    }
private:
    static void run(void* parameter);
    void key_motion();
    void timer_loop_motion();
    void adc_monitor();
    void task_per_second(void);

private:
    TaskHandle_t task_handle_{nullptr};
    KEY_STATE key_last_[KEY_NUM];
    KEY_STATE key_now_[KEY_NUM];
    uint8_t tick[KEY_NUM];
    uint32_t per_second;
};