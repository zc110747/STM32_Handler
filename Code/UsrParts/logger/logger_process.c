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
static uint8_t logger_dev_tx_buffer(uint8_t *ptr, uint8_t size);

uint8_t logger_init(void)
{
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
    if(g_logger_info.protect == NULL) return LOGGER_ERROR;
#endif    

    return LOGGER_OK;
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

static uint8_t logger_protect(void)
{
#if OS_TYPE == OS_TYPE_NONE
    return LOGGER_OK;
#elif OS_TYPE == OS_TYPE_FREEROTPS
    if(g_logger_info.is_multi_thread_run == 0)
    {
       return LOGGER_OK;  
    }
    else
    {
        if(xSemaphoreTake(g_logger_info.protect, (TickType_t)10) == pdPASS)
        {
           return LOGGER_OK;   
        }
    }
    return LOGGER_ERROR;
#endif
}

static uint8_t logger_unprotect(void)
{
#if OS_TYPE == OS_TYPE_NONE
    return LOGGER_OK;
#elif OS_TYPE == OS_TYPE_FREEROTPS
    if(g_logger_info.is_multi_thread_run == 0)
    {
       return LOGGER_OK;  
    }
        
    xSemaphoreGive(g_logger_info.protect);
    return LOGGER_ERROR;
#endif   
}

uint8_t logger_put_rx_buffer(LOG_DEVICE dev, uint8_t *ptr, uint8_t size)
{
    uint32_t rx_allow_size, index;
    
    //device not use as logger interface
    if(dev != g_logger_info.device
    || g_logger_info.is_init != 1 )
        return LOGGER_ERROR;
    
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
            return LOGGER_ERROR;
        }
        
        for(index=0; index<size; index++)
        {
            CircularBufferPut(&g_logger_info.LoggerRxBufferInfo, ptr[index]);
        }
    }
    return LOGGER_OK;
}

uint8_t logger_get_tx_byte(LOG_DEVICE dev, uint8_t *data)
{
    uint8_t c;
    
    //device not use as logger interface
    if(dev != g_logger_info.device
    || g_logger_info.is_init != 1 )
        return LOGGER_ERROR;
    
    if(!CircularBufferIsEmpty(&g_logger_info.LoggerTxBufferInfo))
    {
       CircularBufferGet(&g_logger_info.LoggerTxBufferInfo, c);
       *data = c;
       return LOGGER_OK;
    }
    return LOGGER_ERROR;
}

//only logger buffer is allow send 
//can put, otherwise will discard the data
uint8_t logger_put_tx_buffer(LOG_DEVICE dev, uint8_t *ptr, uint8_t size)
{
    uint32_t tx_allow_size, index;
    
    //device not use as logger interface
    if(dev != g_logger_info.device
    || g_logger_info.is_init != 1 )
        return LOGGER_ERROR;

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
            return LOGGER_ERROR;
        }
        
        for(index=0; index<size; index++)
        {
            CircularBufferPut(&g_logger_info.LoggerTxBufferInfo, ptr[index]);
        }
    }
    return LOGGER_OK;   
}

#define LOGGER_MAX_BUFFER_SIZE      128
static char LoggerMaxBuffer[LOGGER_MAX_BUFFER_SIZE];
int print_log(LOG_LEVEL level, const char* fmt, ...)
{
    int len, bufferlen;
    char *pbuf;
    int outlen = 0;
    
    if(g_logger_info.is_init != 1)
        return -1;

    if(level < g_logger_info.level)
        return -2;

    if(logger_protect() == LOGGER_OK)
    {
        va_list	valist;
        
        len = LOGGER_MAX_BUFFER_SIZE;
        bufferlen = len - 1;
        pbuf = LoggerMaxBuffer;
        
        len = snprintf(pbuf, bufferlen, "level:%d info:", level);
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
        logger_unprotect();
    }
    
    return outlen;
}

static uint8_t logger_dev_tx_buffer(uint8_t *ptr, uint8_t size)
{
    uint8_t res = LOGGER_ERROR;
    
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

__weak uint8_t i2c_logger_write(uint8_t *ptr, uint8_t size)
{  
    return LOGGER_OK;
}

__weak uint8_t uart_logger_write(uint8_t *ptr, uint8_t size)
{ 
    return LOGGER_OK;    
}

__weak uint8_t spi_logger_write(uint8_t *ptr, uint8_t size)
{ 
    return LOGGER_OK;    
}

__weak uint8_t eth_logger_write(uint8_t *ptr, uint8_t size)
{ 
    return LOGGER_OK;    
}
