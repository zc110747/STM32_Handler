//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      application.cpp
//
//  Purpose:
//      application work main workflow.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "application.hpp"
#include "includes.hpp"
#include "schedular.hpp"
#include "monitor.hpp"
#include "i2c_monitor.hpp"
#include "ff.h"
#include "timer_manage.hpp"
#include "fatfs_logs.h"

BYTE work[FF_MAX_SS];

void fatfs_app(void)
{
    FATFS fs;
    FIL fil;
    UINT bw;  
    FRESULT res;
    
    res = f_mount(&fs, "1:", 1);
    if(res == FR_OK)
    {
       goto __mount;
    }
    else
    {
        res = f_mkfs("1:", 0, work, FF_MAX_SS);
        if(res == FR_OK)
        {
            res = f_mount(&fs, "1:", 1);

            if(res == FR_OK)
            {
 __mount:
                res = f_open(&fil, "1:hello.txt", FA_READ | FA_WRITE);
                if(res != FR_OK)
                {
                   PRINT_LOG(LOG_DEBUG, "f_mount open failed!") 
                }
                else
                {
                    f_read(&fil, work, 64, &bw);
                    if(bw != 0)
                    {
                        work[bw] = 0;
                        PRINT_LOG(LOG_DEBUG, "%s", work); 
                    }
                    f_close(&fil);
                }
                f_mount(0, "1:", 0);
            }
            else
            {
               PRINT_LOG(LOG_DEBUG, "f_mount failed:%d", res); 
            }
        }
        else
        {
            PRINT_LOG(LOG_DEBUG, "f_mkfs failed:%d", res); 
        }
    }
}

BaseType_t application_init(void)
{
    BaseType_t xReturn;
    
    //test fatfs application    
    //fatfs_app();
     
    //timer manage.
    xReturn &= SysTimeManage::get_instance()->init();
    
    //schedular task init
    xReturn = schedular::get_instance()->init();

    //motion_manage task init
    xReturn &= monitor_manage::get_instance()->init();
        
    //i2c motion key and output
    xReturn &= i2c_monitor::get_instance()->init();
    
    if(fatfs_logs_init() != FATFS_LOG_OK)
        xReturn = pdFAIL;
        
    return xReturn;
}


