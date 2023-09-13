//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      logger.cpp
//
//  Purpose:
//      logger debug feature, support level, used rtc to show clock.
//      use PRINT_LOG(level, fmt, ...) can printf.
//      example: PRINT_LOG(LOG_DEBUG, "example:%s", "test");
//
// Author:
//      @zc
//
//  Assumptions:
//	
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "logger.h"
#include "driver.hpp"

/// \brief memoryBuffer
/// - memory buffer cache
static char memoryBuffer[LOGGER_MESSAGE_BUFFER_SIZE+1];
static LOGGER_INFO g_logger_info;
static BaseType_t is_os_on = pdFALSE;

static void logger_rx_run(void *parameter);
static void logger_tx_run(void *parameter);

char *get_memory_buffer_pointer(uint16_t size);

BaseType_t logger_init(void)
{
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;
    
    memset(&g_logger_info, 0, sizeof(LOGGER_INFO));
    
    g_logger_info.interface_ = LOGGER_DEFAULT_INTERFACE;
    g_logger_info.memory_start_pointer_ = memoryBuffer;
    g_logger_info.memory_end_pointer_ = &memoryBuffer[LOGGER_MESSAGE_BUFFER_SIZE];
    g_logger_info.logger_init_ok = pdFALSE;
    g_logger_info.log_level_ = LOG_INFO;
    
    //hardware init for logger.
    xReturned = usart_driver_init();
    xReturned &= rtc_driver_init();   
    
    // Create the task, storing the handle
    xReturned &= xTaskCreate(logger_rx_run, "looger rx run", LOGGER_RX_TASK_STACK,                    
                    ( void * ) NULL, LOGGER_RX_TASK_PROITY, &xHandle );     
    xReturned &= xTaskCreate(logger_tx_run, "looger tx run", LOGGER_TX_TASK_STACK,                    
                    ( void * ) NULL, LOGGER_TX_TASK_PROITY, &xHandle );     
    
    // create queue
    g_logger_info.mutex_ = xSemaphoreCreateMutex();
    g_logger_info.tx_queue_ = xQueueCreate(LOGGER_TX_QUEUE_NUM, sizeof(LOG_MESSAGE));
    g_logger_info.rx_queue_ = xQueueCreate(LOGGER_TX_QUEUE_NUM, sizeof(uint8_t));
    if(g_logger_info.mutex_ == NULL 
    || g_logger_info.tx_queue_ == NULL 
    || g_logger_info.rx_queue_ == NULL)
        xReturned = pdFAIL;
    
    if(xReturned == pdPASS)
    {
        g_logger_info.logger_init_ok = pdTRUE;
        PRINT_LOG(LOG_INFO, "logger manage module init success!");
    }
    return xReturned; 
}

char *get_memory_buffer_pointer(uint16_t size)
{
    char *pCurrentMemBuffer;

    pCurrentMemBuffer = g_logger_info.memory_start_pointer_;
    g_logger_info.memory_start_pointer_ = pCurrentMemBuffer+size;
	if(g_logger_info.memory_start_pointer_ >  g_logger_info.memory_end_pointer_)
	{
		pCurrentMemBuffer = memoryBuffer;
		g_logger_info.memory_start_pointer_ = pCurrentMemBuffer + size;
	}
	return(pCurrentMemBuffer);
}

static BaseType_t xLoggerSempTake(uint8_t is_os_on)
{
    BaseType_t xReturn = pdTRUE;
    
    if(is_os_on == pdTRUE)
    {
        xReturn = xSemaphoreTake(g_logger_info.mutex_, ( TickType_t )10);    
    }
    return xReturn;
}

static void xLoggerSempGive(uint8_t is_os_on)
{  
    if(is_os_on == pdTRUE)
    {
        xSemaphoreGive(g_logger_info.mutex_);    
    }
}

int print_log(LOG_LEVEL level, const char* fmt, ...)
{
    int len, bufferlen;
    char *pbuf, *pstart;
    LOG_MESSAGE logger_message_;
    BaseType_t os_on;
    
    if(level < g_logger_info.log_level_)
        return -1;
    
    if(g_logger_info.logger_init_ok != pdTRUE)
        return -2;
    
    os_on = is_os_on;
    
    if(xLoggerSempTake(os_on) == pdTRUE )
    {
        pstart = get_memory_buffer_pointer(LOGGER_MAX_BUFFER_SIZE);
        len = LOGGER_MAX_BUFFER_SIZE;
        bufferlen = len - 1;
        pbuf = pstart;
        logger_message_.length = 0;
        logger_message_.ptr = pstart;
        
        RTC_INFO rtc_info = rtc_get_info();
        len = snprintf(pbuf, bufferlen, "%02d:%02d:%02d level:%d info:",
            rtc_info.time.Hours,
            rtc_info.time.Minutes,
            rtc_info.time.Seconds,
            level);
        
        if((len<=0) || (len>=bufferlen))
        {
            xLoggerSempGive(os_on);
            return -3;
        }

        logger_message_.length += len;
        pbuf = &pbuf[len];
        bufferlen -= len;
        
        va_list	valist;
        va_start(valist, fmt);
        len = vsnprintf(pbuf, bufferlen, fmt, valist);
        va_end(valist);
        xLoggerSempGive(os_on);

        if((len<=0) || (len>=bufferlen))
        {
            return -4;
        }
        
        logger_message_.length += len;
        pbuf = &pbuf[len];
        bufferlen -= len;

        if(bufferlen < 3)
        {
            return -5;
        }
        
        pbuf[0] = '\r';
        pbuf[1] = '\n';
        logger_message_.length += 2;

        xQueueSend(g_logger_info.tx_queue_, &logger_message_, (portTickType)1);
    }
    
    return logger_message_.length;
}

BaseType_t upload_logger_data_isr(uint8_t data)
{
    BaseType_t xReturn;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xReturn = xQueueSendFromISR(g_logger_info.rx_queue_, &data, &xHigherPriorityTaskWoken );

    if( xHigherPriorityTaskWoken )
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    return xReturn;
}

volatile int32_t ITM_RxBuffer = ITM_RXBUFFER_EMPTY;
void logger_rx_run(void *parameter)
{
    uint8_t data;
    uint8_t buffer[256] = {0};
    uint8_t index = 0;
    
    while(1)
    {
        if (xQueueReceive(g_logger_info.rx_queue_, &data, 5 ) == pdPASS)
        {
            if(data != '\n')
            {
                buffer[index++] = data;
            }
            else
            {
                buffer[index] = '\0';
                PRINT_LOG(LOG_INFO, "%s", buffer);
                index = 0;
            }
        }
        else
        {
            while(ITM_CheckChar() == 1)
            {
                data = ITM_ReceiveChar();
                buffer[index++] = data;
                if(data != '\n' && data != '\r')
                {
                    ITM_SendChar(data);
                }
                else
                {
                    buffer[index] = '\0';
                    PRINT_LOG(LOG_INFO, "%s", buffer);
                    index = 0;
                    break;
                }
            }
        }
    }
}

/*
itm is allow:
1.swo pin need link with stlink
2.debug>trace need config
3.read in View/Serial Windows/Debug View
*/
void logger_tx_run(void *parameter)
{
    LOG_MESSAGE msg;
    
    is_os_on = pdTRUE;
    
    while(1)
    {
        if (xQueueReceive(g_logger_info.tx_queue_, &msg, portMAX_DELAY) == pdPASS)
        {
            if(g_logger_info.interface_ == LOGGER_INTERFACE_UART)
            {    
                usart_translate(msg.ptr, msg.length); 
            }
            else if(g_logger_info.interface_ == LOGGER_INTERFACE_SWO)
            {          
                for(int i=0; i<msg.length; i++)
                {
                    ITM_SendChar(msg.ptr[i]);
                }
            }
            else if(g_logger_info.interface_ == LOGGER_INTERFACE_RTT)
            {
                SEGGER_RTT_Write(0, msg.ptr, msg.length); 
            }
            else
            {
                
            }
            vTaskDelay(1);
        }
    }
}
