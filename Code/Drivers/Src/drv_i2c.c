//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_i2c.c
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
#include "drv_i2c.h"

static I2C_HandleTypeDef hi2c2;

BaseType_t i2c_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_I2C2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    /*Configure GPIO pin : PB12 */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); 

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

BaseType_t i2c_write(uint8_t addr, uint8_t data)
{
    if(HAL_I2C_Master_Transmit(&hi2c2, addr | 0x00, &data, 1, PCF8574_I2C_TIMEOUT) != HAL_OK)
        return pdFAIL;

    return pdPASS;    
}

BaseType_t i2c_read(uint8_t addr,uint8_t *pdata)
{
    if(HAL_I2C_Master_Receive(&hi2c2, addr | 0x01, pdata, 1, PCF8574_I2C_TIMEOUT) != HAL_OK)
        return pdFAIL;
    
    return pdPASS;
}

extern void i2c_isr_trigger(void);

void EXTI15_10_IRQHandler(void)
{
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET)
    {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
        
        i2c_isr_trigger();
        
        HAL_NVIC_DisableIRQ(EXTI15_10_IRQn); 
    }
}
