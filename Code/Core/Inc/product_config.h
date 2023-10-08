//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      product_config.h
//
//  Purpose:
//      config file for globa defined.
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _PRODUCT_CONFIG_H
#define _PRODUCT_CONFIG_H

#define LOGGER_ON                       1
#define SUPPORT_FATFS_LOG               1

#define IWDG_MODULE_ON                  0
#define WWDG_MODULE_ON                  0

///driver test define
#define NO_TEST                         0
#define USART_TEST                      1
#define DAC_TEST                        2

#define RUN_TEST_MODE                   NO_TEST

#define LED_TEST                        0
#define SDRAM_TEST                      0
#define LCD_TEST                        1
#define UART_TEST                       0
#define SDMMC_TEST                      0

//i2c workmode
#define I2C_USE_HARDWARE                0
#define I2C_USE_SOFTWARE                1

#define I2C_RUN_MODE                    I2C_USE_SOFTWARE

//default config
#define DAC_DEFAULT_VOL                 1500   //uint:mv

//logger interface
#define LOGGER_INTERFACE_UART           0
#define LOGGER_INTERFACE_SWO            1
#define LOGGER_INTERFACE_RTT            2
#define LOGGER_INTERFACE_NET            3

#define LOGGER_DEFAULT_INTERFACE        LOGGER_INTERFACE_UART

#define TRACE_HARDWARE_VERSION          "V1.0.0"
#define TRACE_SOFTWARE_VERSION          "V0.1.0"


//task define
#define HTTPD_TASK_STACK            512
#define HTTPD_TASK_PROITY           (tskIDLE_PRIORITY+8)

#define SCHEDULAR_TASK_STACK        2048
#define MONITOR_TASK_STACK           512
#define I2C_MONITOR_TASK_STACK       512
#define FATFS_LOG_TASK_STACK         512

#define LWIP_TASK_PROITY            (tskIDLE_PRIORITY+8)
#define SCHEDULAR_TASK_PROITY       (tskIDLE_PRIORITY+2)
#define MONITOR_TASK_PROITY         (tskIDLE_PRIORITY+2)
#define I2C_MONITOR_TASK_PROITY     (tskIDLE_PRIORITY+3)
#define FATFS_LOG_TASK_PROITY       (tskIDLE_PRIORITY+2)

#endif
