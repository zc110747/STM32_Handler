//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      i2c.c
//      hardware: 
//          I2C2_SCL ------------ PH4
//          I2C2_SDA ------------ PH5
//          EXIT ---------------- PB12
//  Purpose:
//     i2c driver.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "i2c.h"

static I2C_HandleTypeDef i2c2_handler_;

static BaseType_t i2c_hardware_init(void);
static BaseType_t i2c_test(void);

BaseType_t i2c_init(void)
{
    BaseType_t result;

    result = i2c_hardware_init();
    
    if(result == pdPASS)
    {
        i2c_test();
    }
    else
    {
        printf("i2c_driver hardware_init failed\r\n");
    }
    return result;    
}

BaseType_t i2c_write(uint8_t addr, uint8_t data)
{
    if(HAL_I2C_Master_Transmit(&i2c2_handler_, addr | 0x00, &data, 1, PCF8574_I2C_TIMEOUT) != HAL_OK)
        return pdFAIL;

    return pdPASS;    
}

BaseType_t i2c_read(uint8_t addr,uint8_t *pdata)
{
    if(HAL_I2C_Master_Receive(&i2c2_handler_, addr | 0x01, pdata, 1, PCF8574_I2C_TIMEOUT) != HAL_OK)
        return pdFAIL;
    
    return pdPASS;
}

extern void i2c_monitor_trigger_read(void);

void EXTI15_10_IRQHandler(void)
{
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET)
    {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
        
        i2c_monitor_trigger_read();
    }
}
    
static BaseType_t i2c_hardware_init()
{
    i2c2_handler_.Instance = I2C2;
    i2c2_handler_.Init.ClockSpeed = 100000;
    i2c2_handler_.Init.DutyCycle = I2C_DUTYCYCLE_2;
    i2c2_handler_.Init.OwnAddress1 = 0;
    i2c2_handler_.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    i2c2_handler_.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    i2c2_handler_.Init.OwnAddress2 = 0;
    i2c2_handler_.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    i2c2_handler_.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&i2c2_handler_) != HAL_OK)
        return pdFAIL;

    /** Configure Analogue filter
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&i2c2_handler_, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
        return pdFAIL;

    /** Configure Digital filter
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&i2c2_handler_, 0) != HAL_OK)
        return pdFAIL;
    
    return pdPASS;
}

static BaseType_t i2c_test()
{
   return pdPASS;
}
