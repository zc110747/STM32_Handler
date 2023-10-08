//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      fatfs_log.h
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
#ifndef _FATFS_LOG_H
#define _FATFS_LOG_H

#include "main.h"
#include "ff.h"

#define FATFS_SYNC_MAX_TIMES            100
#define FATFS_MAX_MESSAGE_BUFFER        4096
#define FATFS_SIGNAL_WRITE_MAX_SIZE     255
#define FATFS_WAIT_MAX_TIME             64      //wait max timer for fatfs write

#define FATFS_LOG_SYS                   "1:"

#define FATFS_LOG_OK                    0
#define FATFS_LOG_MOUNT_ERROR           1
#define FATFS_LOG_FILE_ERROR            2
#define FATFS_LOG_SEEK_ERROR            3
#define FATFS_LOG_NO_FILE_ERROR         4
#define FATFS_LOG_WRITE_ERROR           5
#define FATFS_LOG_OS_ERROR              6

#define FATFS_LOG_IDLE                  0
#define FATFS_LOG_OPEN                  1
#define FATFS_LOG_CLOSE                 2

#define FATFS_EVENT_WRITE               0
#define FATFS_EVENT_CLOSE               1

#ifdef __cplusplus
extern "C" {
#endif

uint8_t fatfs_logs_init(void);
uint8_t fatfs_close(void);
uint8_t fatfs_write(uint8_t *ptr, uint8_t size);
    
#ifdef __cplusplus
}
#endif   
    
#endif
