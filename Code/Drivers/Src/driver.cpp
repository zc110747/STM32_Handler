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
#include "driver.hpp"
#include "sdram.hpp"
#include "lcd.hpp"
#include "sdmmc.hpp"

void wq_application(void);

BaseType_t driver_init(void)
{
    BaseType_t result = pdPASS;
    
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    //usart init
    //usart first init for logger.
    result = usart_driver_init();
    
    //led init
    //all io clock init in this function, so need the first execute.
    result &= led_driver_init();

    //sdram init
    result &= sdram_driver::get_instance()->init();

    //lcd init
    result &= lcd_driver::get_instance()->init();

    //adc init
    result &= adc_init();

    //key init
    result &= key_init();

    //rng
    result &= alg_driver_init();

    //tpad 
    result &= tpad_driver_init();
    result &= pwm_driver_init();
    
    //rtc
    result &= rtc_driver_init();
    
    //i2c
    result &= i2c_init();
    
    //dac
    result &= dac_init();
    
    //sdmmc
    result &= sdmmc_driver::get_instance()->init();
    
    //spi
    result &= spi_init();
    
    //dfu test
    dsp_app();
    
    //wq test
    wq_init();
    wq_application();
    
    //dma 
    dma_init();

    result &= wdg_driver_init();
    iwdg_reload();
    
    return result;
}

void wq_application(void)
{
    static uint8_t buffer[256];
    const char *ptr = "spi read write test success!\r\n";
    
    //spi-wq erase/write
//    memcpy(buffer, ptr, strlen(ptr));
//    wq_erase_sector(0);
//    wq_write_page(buffer, 0, strlen(ptr));
    
    memset(buffer, 0, 256);
    wq_read(buffer, 0, strlen(ptr));
    
    printf("%s", buffer);
}

HAL_StatusTypeDef read_disk(uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks)
{
    return sdmmc_driver::get_instance()->read_disk(buf, startBlocks, NumberOfBlocks);
}

HAL_StatusTypeDef write_disk(const uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks)
{
    return sdmmc_driver::get_instance()->write_disk(buf, startBlocks, NumberOfBlocks);
}