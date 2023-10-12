//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      driver.cpp
//
//  Purpose:
//      driver defined for init.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "driver.h"

void wq_application(void);

BaseType_t driver_init(void)
{
    BaseType_t result = pdPASS;
    
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    result &= rtc_driver_init();
    
    //led init
    //all io clock init in this function, so need the first execute.
    result &= led_driver_init();

    //sdram init
    result &= sdram_driver_init();

    //lcd init
    result &= lcd_driver_init();

    //adc init
    result &= adc_driver_init();

    //key init
    result &= key_driver_init();

    //rng
    result &= alg_driver_init();

    //tpad 
    result &= tpad_driver_init();
    result &= pwm_driver_init();
    result &= timer_extend_driver_init();
    
    //i2c
    result &= pcf8574_driver_init();
    result &= ap3216_driver_init();
    
    //dac
    result &= dac_driver_init();
    
    //sdcard init
    result &= sdcard_driver_init();
    
    //dma 
    result &= dma_driver_init();
    
    //wq test
    result &= wq25_driver_init();
    
    //watchdog enable
    result &= wdg_driver_init();
    
    iwdg_reload();
    
    return result;
}

void wq_application(void)
{
    static uint8_t buffer[256];
    const char *ptr = "spi read write test success!";
    
    //spi-wq erase/write
//    memcpy(buffer, ptr, strlen(ptr));
//    wq_erase_sector(0);
//    wq_write_page(buffer, 0, strlen(ptr));
    
    memset(buffer, 0, 256);
    
    if(wq_memory_read(0, buffer,strlen(ptr)) == WQ_OP_OK)
    {  
        PRINT_LOG(LOG_INFO, "%s", buffer);
    }
}
