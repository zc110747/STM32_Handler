//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      logger_process.h
//
//  Purpose:
//     
//
// Author:
//      @zc
//
// Revision History:
//      Version V1.0b2 Create.
/////////////////////////////////////////////////////////////////////////////
#ifndef _LOOGGER_PROCESS_H
#define _LOOGGER_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "circular_buffer.h"

//#define GLOBAL_OK               0
//#define GLOBAL_ERROR            1

#define OS_TYPE_NONE            0
#define OS_TYPE_FREEROTPS       1
#define OS_TYPE                 OS_TYPE_FREEROTPS

#define DEFAULT_LOGGER_DEV      LOG_DEVICE_USART
#define DEFAULT_LOGGER_LEVEL    LOG_DEBUG

#define LOGGER_BUFFER_SIZE      2048
typedef enum
{
	LOG_TRACE = 0,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
}LOG_LEVEL;

//define LOG wheather be record
#define LOG_RECORD              (1<<7)

//only level more than info can record
#define LOG_INFO_RECORD         (LOG_INFO | LOG_RECORD)
#define LOG_WARN_RECORD         (LOG_WARN | LOG_RECORD)
#define LOG_ERROR_RECORD        (LOG_ERROR | LOG_RECORD)
#define LOG_FATAL_RECORD        (LOG_FATAL | LOG_RECORD)

//current used logger interface
//only one support at one time
typedef enum
{
    LOG_DEVICE_NONE = 0,
    LOG_DEVICE_I2C,
    LOG_DEVICE_SPI,
    LOG_DEVICE_USART,
    LOG_DEVICE_ETH,
}LOG_DEVICE;

GlobalType_t logger_init(void);
void logger_set_multi_thread_run(void); //call when multi thread run, then used mutex protect
void logger_process_run(void);
GlobalType_t logger_put_tx_buffer(LOG_DEVICE dev, uint8_t *ptr, uint8_t size);
GlobalType_t logger_get_tx_byte(LOG_DEVICE dev, uint8_t *data);
GlobalType_t logger_put_rx_buffer(LOG_DEVICE dev, uint8_t *ptr, uint8_t size);
int print_log(LOG_LEVEL level, const char* fmt, ...);

//interface need support other device
GlobalType_t i2c_logger_write(uint8_t *ptr, uint8_t size);
GlobalType_t spi_logger_write(uint8_t *ptr, uint8_t size);
GlobalType_t uart_logger_write(uint8_t *ptr, uint8_t size);
GlobalType_t eth_logger_write(uint8_t *ptr, uint8_t size);
    
#if LOGGER_ON == 1
#define PRINT_LOG(level, fmt, ...)       print_log((LOG_LEVEL)level, fmt, ##__VA_ARGS__);
#else
#define PRINT_LOG(level, fmt, ...)          
#endif
#ifdef __cplusplus
}
#endif 

#endif
