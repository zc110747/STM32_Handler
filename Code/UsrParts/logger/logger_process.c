//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      logger_process.c
//
//  Purpose:
//     
//
// Author:
//      @zc
//
// Revision History:
//      Version V1.0b1 Create.
/////////////////////////////////////////////////////////////////////////////
#include "logger_process.h"
#include "cmd_process.h"
#include "driver.h"

typedef struct
{
    //logger rx buffer
    volatile CircularBuffer LoggerRxBufferInfo;
    uint8_t LoggerRxBuffer[LOGGER_BUFFER_SIZE];
    
    //logger tx buffer
    volatile CircularBuffer LoggerTxBufferInfo;
    uint8_t LoggerTxBuffer[LOGGER_BUFFER_SIZE];
   
    uint8_t is_init;
    
    LOG_LEVEL level;
 
    LOG_DEVICE device;

    uint8_t is_multi_thread_run;
    
#if OS_TYPE == OS_TYPE_FREEROTPS 
    SemaphoreHandle_t protect;
#endif
}LOGGER_INFO;

static LOGGER_INFO g_logger_info = {0};
static GlobalType_t logger_dev_tx_buffer(uint8_t *ptr, uint8_t size);
static GlobalType_t logger_protect(void);
static GlobalType_t logger_unprotect(void);

GlobalType_t logger_init(void)
{
    BaseType_t xReturned;
 
    //uart driver init for logger
    xReturned = usart_driver_init();
    if(xReturned != HAL_OK)
    {
        return GLOBAL_ERROR;
    }
    
    CircularBufferInit(&g_logger_info.LoggerTxBufferInfo, LOGGER_BUFFER_SIZE, g_logger_info.LoggerTxBuffer);
    CircularBufferInit(&g_logger_info.LoggerRxBufferInfo, LOGGER_BUFFER_SIZE, g_logger_info.LoggerRxBuffer);
    
    //support command init
    cmd_protocol_init();
    
    g_logger_info.is_init = 1;
    g_logger_info.level = DEFAULT_LOGGER_LEVEL;
    g_logger_info.device = DEFAULT_LOGGER_DEV;
    g_logger_info.is_multi_thread_run = 0;

#if OS_TYPE == OS_TYPE_FREEROTPS 
    g_logger_info.protect = xSemaphoreCreateMutex();
    if(g_logger_info.protect == NULL) return GLOBAL_ERROR;
#endif    

    return GLOBAL_OK;
}

void logger_process_run(void)
{
    uint8_t c;
    
    if(g_logger_info.is_init == 1)
    {
        if(!CircularBufferIsEmpty(&g_logger_info.LoggerRxBufferInfo))
        {
            CircularBufferGet(&g_logger_info.LoggerRxBufferInfo, c);
            
            cmd_process(c);
        }
    }
}

void logger_set_multi_thread_run(void)
{
    g_logger_info.is_multi_thread_run = 1;
}

GlobalType_t logger_put_rx_buffer(LOG_DEVICE dev, uint8_t *ptr, uint8_t size)
{
    uint32_t rx_allow_size, index;
    
    //device not use as logger interface
    if(dev != g_logger_info.device
    || g_logger_info.is_init != 1 )
        return GLOBAL_ERROR;
    
    if(size == 1)
    {
       //if size is 1, not check for efficiency because the macro support check
       CircularBufferPut(&g_logger_info.LoggerRxBufferInfo, ptr[0]);
    }
    else
    {
        rx_allow_size = LOGGER_BUFFER_SIZE - CircularBufferSize(&g_logger_info.LoggerRxBufferInfo);
        if(size > rx_allow_size)
        {
            return GLOBAL_ERROR;
        }
        
        for(index=0; index<size; index++)
        {
            CircularBufferPut(&g_logger_info.LoggerRxBufferInfo, ptr[index]);
        }
    }
    return GLOBAL_OK;
}

GlobalType_t logger_get_tx_byte(LOG_DEVICE dev, uint8_t *data)
{
    uint8_t c;
    
    //device not use as logger interface
    if(dev != g_logger_info.device
    || g_logger_info.is_init != 1 )
        return GLOBAL_ERROR;
    
    if(!CircularBufferIsEmpty(&g_logger_info.LoggerTxBufferInfo))
    {
       CircularBufferGet(&g_logger_info.LoggerTxBufferInfo, c);
       *data = c;
       return GLOBAL_OK;
    }
    return GLOBAL_ERROR;
}

//only logger buffer is allow send 
//can put, otherwise will discard the data
GlobalType_t logger_put_tx_buffer(LOG_DEVICE dev, uint8_t *ptr, uint8_t size)
{
    uint32_t tx_allow_size, index;
    
    //device not use as logger interface
    if(dev != g_logger_info.device
    || g_logger_info.is_init != 1 )
        return GLOBAL_ERROR;

    if(size == 1)
    {
       //if size is 1, not check for efficiency because the macro support check
       CircularBufferPut(&g_logger_info.LoggerTxBufferInfo, ptr[0]);
    }
    else
    {
        tx_allow_size = LOGGER_BUFFER_SIZE - CircularBufferSize(&g_logger_info.LoggerTxBufferInfo);
        if(size > tx_allow_size)
        {
            return GLOBAL_ERROR;
        }
        
        for(index=0; index<size; index++)
        {
            CircularBufferPut(&g_logger_info.LoggerTxBufferInfo, ptr[index]);
        }
    }
    return GLOBAL_OK;   
}

#if SUPPORT_FATFS_LOG == 1
#include "fatfs_logs.h"
#endif

#define LOGGER_MAX_BUFFER_SIZE      128
static char LoggerMaxBuffer[LOGGER_MAX_BUFFER_SIZE];
int print_log(LOG_LEVEL level, const char* fmt, ...)
{
    int len, bufferlen;
    char *pbuf;
    int outlen = 0;
    
    if(g_logger_info.is_init != 1)
        return -1;

    if((level&(~LOG_RECORD)) < g_logger_info.level)
        return -2;

    if(logger_protect() == GLOBAL_OK)
    {
        va_list	valist;
        
        len = LOGGER_MAX_BUFFER_SIZE;
        bufferlen = len - 1;
        pbuf = LoggerMaxBuffer;
        
        len = snprintf(pbuf, bufferlen, "level:%d info:", (level&(~LOG_RECORD)));
        if((len<=0) || (len>=bufferlen))
        {
            logger_unprotect();
            return -3;
        }
        outlen += len;
        pbuf = &pbuf[len];
        bufferlen -= len;
        
        va_start(valist, fmt);
        len = vsnprintf(pbuf, bufferlen, fmt, valist);
        va_end(valist);
        if((len<=0) || (len>=bufferlen))
        {
            logger_unprotect();
            return -4;
        } 
        outlen += len;
        pbuf = &pbuf[len];
        bufferlen -= len;
        
        if(bufferlen < 3)
        {
            logger_unprotect();
            return -4;
        }
        
        pbuf[0] = '\r';
        pbuf[1] = '\n';
        outlen += 2;
        logger_dev_tx_buffer((uint8_t *)LoggerMaxBuffer, outlen);
#if SUPPORT_FATFS_LOG == 1
        if(level&LOG_RECORD)
        {
            fatfs_write((uint8_t *)LoggerMaxBuffer, outlen);
        }
#endif
        logger_unprotect();
    }
    
    return outlen;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//local function
/////////////////////////////////////////////////////////////////////////////////////////////////////////
static GlobalType_t logger_protect(void)
{
#if OS_TYPE == OS_TYPE_NONE
    return GLOBAL_OK;
#elif OS_TYPE == OS_TYPE_FREEROTPS
    if(g_logger_info.is_multi_thread_run == 0)
    {
       return GLOBAL_OK;  
    }
    else
    {
        if(xSemaphoreTake(g_logger_info.protect, (TickType_t)10) == pdPASS)
        {
           return GLOBAL_OK;   
        }
    }
    return GLOBAL_ERROR;
#endif
}

static GlobalType_t logger_unprotect(void)
{
#if OS_TYPE == OS_TYPE_NONE
    return GLOBAL_OK;
#elif OS_TYPE == OS_TYPE_FREEROTPS
    if(g_logger_info.is_multi_thread_run == 0)
    {
       return GLOBAL_OK;  
    }
        
    xSemaphoreGive(g_logger_info.protect);
    return GLOBAL_ERROR;
#endif   
}

static GlobalType_t logger_dev_tx_buffer(uint8_t *ptr, uint8_t size)
{
    GlobalType_t res = GLOBAL_ERROR;
    
    switch(g_logger_info.device)
    {
        case LOG_DEVICE_I2C:
            res = i2c_logger_write(ptr, size);
            break;
        case LOG_DEVICE_SPI:
            res = spi_logger_write(ptr, size);
            break;
        case LOG_DEVICE_USART:
            res = uart_logger_write(ptr, size);
            break;
        case LOG_DEVICE_ETH:
            res = eth_logger_write(ptr, size);
            break;
        default:
            break;
    }
    return res;
}

__weak GlobalType_t i2c_logger_write(uint8_t *ptr, uint8_t size)
{  
    return GLOBAL_OK;
}

__weak GlobalType_t uart_logger_write(uint8_t *ptr, uint8_t size)
{ 
    return GLOBAL_OK;    
}

__weak GlobalType_t spi_logger_write(uint8_t *ptr, uint8_t size)
{ 
    return GLOBAL_OK;    
}

__weak GlobalType_t eth_logger_write(uint8_t *ptr, uint8_t size)
{ 
    return GLOBAL_OK;    
}
