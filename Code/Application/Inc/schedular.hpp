//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      schedular.hpp
//
//  Purpose:
//      schedular run task.
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

#include "includes.hpp"

class schedular
{
public:
    static schedular *get_instance()
    {
        static schedular instance_;
        return &instance_;
    }
    bool init(void);
    
private:
    static void run(void* parameter);
    
private:
    IWDG_HandleTypeDef hiwdg;

    TaskHandle_t task_handle_{nullptr};
};
