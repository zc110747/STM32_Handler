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

typedef struct
{
    uint8_t state;
    
    uint8_t is_os_run;
    
    uint16_t w_times;
    
    SemaphoreHandle_t xMutexM;
    
    MessageBufferHandle_t xEventMsgBuffer;
    
    /* fatfs information */
    FATFS fs;
    
    FIL f_file;
    
    char file_name[20];
    
    UINT pw;
}FATFS_LOG_INFO;

static uint8_t is_fatfs_os_on = 0;
static FATFS_LOG_INFO gLoggerInfo = {0};
static RTC_INFO gRtcInfo;

//local function
static uint8_t fatfs_device_close(void);
static uint8_t fatfs_device_write(uint8_t *ptr, uint8_t size);
static void fatfs_log_task(void *parameter);
static uint8_t fatfs_os_init(void);
static size_t fatfs_send_message(uint8_t *pMsg, uint8_t size);

uint8_t fatfs_logs_init(void)
{
    FRESULT f_res;
    
    memset(&gLoggerInfo, 0, sizeof(FATFS_LOG_INFO));
    gLoggerInfo.state = FATFS_LOG_CLOSE;
    
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
    
    if(fatfs_os_init() != pdPASS)
        return FATFS_LOG_OS_ERROR;

    gLoggerInfo.state = FATFS_LOG_OPEN;
    return FATFS_LOG_OK;
}

static uint8_t fatfs_os_init(void)
{
    BaseType_t xReturn;
    TaskHandle_t xTaskHanler;
    
    
	xReturn = xTaskCreate( fatfs_log_task,
							(const char *)"fatfs_log_task",
							FATFS_LOG_TASK_STACK,
							NULL,
							FATFS_LOG_TASK_PROITY,
							&xTaskHanler);
                            
    gLoggerInfo.xEventMsgBuffer = xMessageBufferCreate(FATFS_MAX_MESSAGE_BUFFER);
    gLoggerInfo.xMutexM = xSemaphoreCreateMutex();
    
    if(xReturn != pdPASS 
    || gLoggerInfo.xEventMsgBuffer == NULL
    || gLoggerInfo.xMutexM == NULL)
        return pdFAIL;
    
    return pdPASS;
}

uint8_t fatfs_write(uint8_t *ptr, uint8_t size)
{
    uint8_t status, total_size;
    uint8_t *pbuffer;
    pbuffer = pvPortMalloc(FATFS_SIGNAL_WRITE_MAX_SIZE);
    
    rtc_get_info(&gRtcInfo);
    if(is_fatfs_os_on == 0)
    {
        sprintf((char *)pbuffer, "%02d-%02d-%02d ", 
            gRtcInfo.time.Hours, gRtcInfo.time.Minutes, gRtcInfo.time.Seconds);
        total_size = strlen((char *)pbuffer);
        memcpy((char *)&pbuffer[total_size], ptr, size);
        total_size += size;
        
        //if os not on, just write to the fatfs system
        status = fatfs_device_write((uint8_t *)pbuffer, total_size);
    }
    else
    {
        pbuffer[0] = FATFS_EVENT_WRITE;
        total_size = 1;
        sprintf((char *)&pbuffer[1], "%02d-%02d-%02d ", 
            gRtcInfo.time.Hours, gRtcInfo.time.Minutes, gRtcInfo.time.Seconds);
        total_size += strlen((char *)&pbuffer[1]);
        memcpy((char *)&pbuffer[total_size], ptr, size);
        total_size += size;
        
        if(fatfs_send_message(pbuffer, size) == 0)
        {
            status = FATFS_LOG_WRITE_ERROR;
        }
    }
    
    vPortFree(pbuffer);
    return status;
}

uint8_t fatfs_close(void)
{
    uint8_t status;

    if(is_fatfs_os_on == 0)
    {
        status = fatfs_device_close();
    }
    else
    {
        uint8_t buffer = FATFS_EVENT_CLOSE;
        if(fatfs_send_message(&buffer, 1) == 0)
        {
            status = FATFS_LOG_WRITE_ERROR;
        }
    }        
    return status;
}

static size_t fatfs_send_message(uint8_t *pMsg, uint8_t size)
{
    size_t bytesent = 0;
    //mutual opeartion as multi end will use this buffer
    BaseType_t ret = xSemaphoreTake(gLoggerInfo.xMutexM, FATFS_WAIT_MAX_TIME) ;
    if(ret== pdPASS)
    {
    	bytesent = xMessageBufferSend(gLoggerInfo.xEventMsgBuffer, pMsg, size, FATFS_WAIT_MAX_TIME);

    }
    xSemaphoreGive(gLoggerInfo.xMutexM);
    return bytesent;
}

static uint8_t fatfs_device_close(void)
{
    FRESULT f_res;
    
    if(gLoggerInfo.state == FATFS_LOG_IDLE)
        return FATFS_LOG_FILE_ERROR;

    gLoggerInfo.state = FATFS_LOG_CLOSE;
    
    f_res = f_close(&gLoggerInfo.f_file);
    return f_res;
}

static uint8_t fatfs_device_write(uint8_t *ptr, uint8_t size)
{
    FRESULT f_res;
    
    if(gLoggerInfo.state == FATFS_LOG_IDLE)
        return FATFS_LOG_FILE_ERROR;
    
    if(gLoggerInfo.state == FATFS_LOG_CLOSE)
    {
        f_res = f_open(&gLoggerInfo.f_file, gLoggerInfo.file_name, FA_WRITE);
        if(f_res != FR_OK)
            return FATFS_LOG_FILE_ERROR;
        
        f_res = f_lseek(&gLoggerInfo.f_file, f_size(&gLoggerInfo.f_file));
        if(f_res != FR_OK)
            return FATFS_LOG_SEEK_ERROR;
    }
    
    f_res = f_write(&gLoggerInfo.f_file, ptr, size, &gLoggerInfo.pw);
    if(f_res != FR_OK)
        return FATFS_LOG_WRITE_ERROR;
    
    gLoggerInfo.w_times++;
    if(gLoggerInfo.w_times > FATFS_SYNC_MAX_TIMES)
    {
        f_sync(&gLoggerInfo.f_file);
        gLoggerInfo.w_times = 0;
    }
    
    return FATFS_LOG_OK;
}

static uint8_t rxBufferM[FATFS_SIGNAL_WRITE_MAX_SIZE];
static void fatfs_log_task(void *parameter)
{
    is_fatfs_os_on = 1;
    size_t rxLen = 0;
    
    while(1)
    {
        rxLen = xMessageBufferReceive(gLoggerInfo.xEventMsgBuffer, rxBufferM, 
                                    FATFS_SIGNAL_WRITE_MAX_SIZE, 
                                    portMAX_DELAY);
        if(rxLen )
        {
            switch(rxBufferM[0])
            {
                case FATFS_EVENT_WRITE:
                    if(fatfs_device_write(&rxBufferM[1], rxLen-1) == FATFS_LOG_OK)
                    {
                        PRINT_LOG(LOG_INFO, "fatfs write success, size:%d!", rxLen-1);
                    }
                    else
                    {
                        PRINT_LOG(LOG_INFO, "fatfs write failed!");
                    }
                    break;
                case FATFS_EVENT_CLOSE:
                    fatfs_device_close();
                    PRINT_LOG(LOG_INFO, "fatfs close!");
                    break;
            }
        }
    }
}