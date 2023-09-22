//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     drv_i2c_mpu9250.h
//
//  Purpose:
//     i2c mpu9250 driver.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_i2c_mpu9250.h"
#include "drv_soft_i2c.h"

#if I2C_RUN_MODE == I2C_USE_HARDWARE
static I2C_HandleTypeDef hi2c2;

BaseType_t mpu9250_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_I2C2_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    hi2c2.Instance = I2C2;
    hi2c2.Init.ClockSpeed = 100000;
    hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c2) != HAL_OK)
        return pdFAIL;

    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
        return pdFAIL;

    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
        return pdFAIL;


    return pdPASS;  
}

BaseType_t mpu9250_i2c_multi_write(uint8_t reg, uint8_t *data, uint8_t size)
{
    uint8_t res;
    
    res = HAL_I2C_Mem_Write(&hi2c2, MPU9250_ADDR, reg, 1, data, size, MPU9250_TIMEOUT);  
    
    if(res != HAL_OK)
        return pdFAIL;
    
    return pdPASS;
}

BaseType_t mpu9250_i2c_multi_read(uint8_t reg, uint8_t *rdata, uint8_t size)
{
    uint8_t res;
    
    res = HAL_I2C_Mem_Read(&hi2c2, MPU9250_ADDR, reg, 1, rdata, size, MPU9250_TIMEOUT);  
    
    if(res != HAL_OK)
        return pdFAIL; 
    
    return pdPASS;
}
#else
//I2C_SCL: PH4
//I2C_SDA: PH5
//I2C_INT: PB12
BaseType_t mpu9250_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    SOFT_I2C_INFO I2C_Info = {0};
    BaseType_t res = pdPASS;
    
    //clock need enable before software i2c Init
    __HAL_RCC_GPIOB_CLK_ENABLE(); 
    __HAL_RCC_GPIOH_CLK_ENABLE(); 

    I2C_Info.scl_pin = GPIO_PIN_4;
    I2C_Info.scl_port = GPIOH;
    I2C_Info.sda_pin = GPIO_PIN_5;
    I2C_Info.sda_port = GPIOH;
    
    if(i2c_soft_init(SOFT_I2C2, &I2C_Info) != I2C_OK)
    {
        return pdFAIL;
    }
    
    
    return pdPASS;  
}

BaseType_t mpu9250_i2c_multi_write(uint8_t reg, uint8_t *data, uint8_t size)
{
    uint8_t res;
    
    portENTER_CRITICAL();
    res = i2c_write_memory(SOFT_I2C2, MPU9250_ADDR, reg, 1, data, size);
    portEXIT_CRITICAL();    
    
    if(res != I2C_OK)
        return pdFAIL;
    
    return pdPASS;
}

BaseType_t mpu9250_i2c_multi_read(uint8_t reg, uint8_t *rdata, uint8_t size)
{
    uint8_t res;
    
    portENTER_CRITICAL();
    res = i2c_read_memory(SOFT_I2C2, MPU9250_ADDR, reg, 1, rdata, size);
    portEXIT_CRITICAL();    
    
    if(res != I2C_OK)
        return pdFAIL;
    
    return pdPASS;
}
#endif

BaseType_t mpu9250_i2c_read_reg(uint8_t reg, uint8_t data)
{
    return mpu9250_i2c_multi_write(reg, &data, 1);
}

BaseType_t mpu9250_i2c_write_reg(uint8_t reg, uint8_t data)
{
    return mpu9250_i2c_multi_write(reg, &data, 1);
}

