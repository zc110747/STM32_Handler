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

#define FATFS_LOG_SYS               "1:"

#define FATFS_LOG_OK                0
#define FATFS_LOG_MOUNT_ERROR       1
#define FATFS_LOG_FILE_ERROR        2
#define FATFS_LOG_SEEK_ERROR        3

#define FATFS_LOG_IDLE              0
#define FATFS_LOG_OPEN              1
#define FATFS_LOG_CLOSE             2

typedef struct
{
    uint8_t state;
    
    uint8_t is_os_run;
    
    FATFS fs;
    
    FIL f_file;
    
    char file_name[20];
}FATFS_LOG_INFO;

#ifdef __cplusplus
extern "C" {
#endif

    
#ifdef __cplusplus
}
#endif   
    
#endif
