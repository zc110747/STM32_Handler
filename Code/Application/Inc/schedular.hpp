
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
