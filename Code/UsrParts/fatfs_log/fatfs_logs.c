//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      fatfs_log.c
//
//  Purpose:
//      
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "fatfs_logs.h"
#include "driver.hpp"

static FATFS_LOG_INFO gLoggerInfo = {0};
static RTC_INFO gRtcInfo;

uint8_t fatfs_logs_init(void)
{
    FRESULT f_res;
    
    memset(&gLoggerInfo, 0, sizeof(FATFS_LOG_INFO));
    gLoggerInfo.state = FATFS_LOG_IDLE;
    
    f_res = f_mount(&gLoggerInfo.fs, FATFS_LOG_SYS, 1);
    if(f_res != FR_OK)
        return FATFS_LOG_MOUNT_ERROR;
    
    //choose the logger file name
    rtc_get_info(&gRtcInfo);
    sprintf(gLoggerInfo.file_name, "%02d-%02d-%02d.txt", 
        gRtcInfo.date.Year, gRtcInfo.date.Month, gRtcInfo.date.Date);
    
    //open the logger file
    f_res = f_open(&gLoggerInfo.f_file, gLoggerInfo.file_name, FA_WRITE);
    if(f_res == FR_NO_FILE)
    {
        f_res = f_open(&gLoggerInfo.f_file, gLoggerInfo.file_name, FA_CREATE_NEW | FA_WRITE);
        if(f_res != FR_OK)
            return FATFS_LOG_FILE_ERROR;
    }
    else if(f_res == FR_OK)
    {
        f_res = f_lseek(&gLoggerInfo.f_file, f_size(&gLoggerInfo.f_file));
        if(f_res != FR_OK)
            return FATFS_LOG_SEEK_ERROR;
    }
    else
        return FATFS_LOG_FILE_ERROR;
    
    f_close(&gLoggerInfo.f_file);
    gLoggerInfo.state = FATFS_LOG_OPEN;
    return FATFS_LOG_OK;
}