//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_soft_i2c.c
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
#include "drv_soft_i2c.h"
#include "drv_i2c.h"

#define I2C_DELAY_COUNT 10

#if I2C_RUN_MODE == I2C_USE_SOFTWARE

BaseType_t i2c_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_I2C2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    I2C_SCL_H();
    I2C_SDA_H();
    
    GPIO_InitStruct.Pin = I2C_SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(I2C_SCL_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(I2C_SDA_PORT, &GPIO_InitStruct);
    
    /*Configure GPIO pin : PB12 */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); 
    
    return pdPASS;  
}

BaseType_t i2c_write(uint8_t addr, uint8_t data)
{
    uint8_t res;
    
    portENTER_CRITICAL();
    res = i2c_write_device(addr, data);
    portEXIT_CRITICAL();
    
    if(res != 0)
    {
        return pdFAIL;
    }
    
    return pdPASS;    
}

BaseType_t i2c_read(uint8_t addr,uint8_t *pdata)
{
    uint8_t res;
    
    portENTER_CRITICAL();
    res = i2c_read_device(addr, pdata);
    portEXIT_CRITICAL();
    
    if(res != 0)
    {
        return pdFAIL;
    }
    
    return pdPASS;  
}
#endif


static void i2c_delay(uint32_t count)
{
    unsigned int i, j;

    for(i=0; i<count; i++)
    {
        for(j=0; j<3; j++)
        {
        }
    }    
}

static void i2c_long_delay(uint32_t count)
{
    unsigned int i, j;

    for(i=0; i<count; i++)
    {
        i2c_delay(1000);
    }
}

static void i2c_sda_config_in(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;  

    GPIO_InitStruct.Pin = I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_SDA_PORT, &GPIO_InitStruct);
}

static void i2c_sda_config_out(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;  

    GPIO_InitStruct.Pin = I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_SDA_PORT, &GPIO_InitStruct);  
}

void i2c_start(void)
{
    i2c_sda_config_out();

    I2C_SDA_H();
    i2c_delay(I2C_DELAY_COUNT);
    I2C_SCL_H();
    i2c_delay(I2C_DELAY_COUNT);
    I2C_SDA_L();
    i2c_delay(I2C_DELAY_COUNT);
    I2C_SCL_L();               
}

void i2c_send_byte(uint8_t byte)
{
    uint8_t index;

    i2c_sda_config_out();
    
    for(index=0; index<8; index++)
    {
        i2c_delay(I2C_DELAY_COUNT);
        if(byte & 0x80)
        {
            I2C_SDA_H();
        }
        else
        {
            I2C_SDA_L();
        }
        byte <<= 1;
        i2c_delay(I2C_DELAY_COUNT);
        I2C_SCL_H();
        i2c_delay(I2C_DELAY_COUNT);
        I2C_SCL_L();   
    }
}

void i2c_send_nack(void)
{
    i2c_sda_config_out();
    i2c_delay(I2C_DELAY_COUNT);
    I2C_SDA_L();
    i2c_delay(I2C_DELAY_COUNT);
    I2C_SCL_H();    
    i2c_delay(I2C_DELAY_COUNT);
    I2C_SCL_L();
}

void i2c_send_ack(void)
{
    i2c_sda_config_out();
    i2c_delay(I2C_DELAY_COUNT);
    I2C_SDA_H();
    i2c_delay(I2C_DELAY_COUNT);
    I2C_SCL_H();    
    i2c_delay(I2C_DELAY_COUNT);
    I2C_SCL_L();
}

uint8_t i2c_read_byte(uint8_t ack_status)
{
    uint8_t index, value = 0;

    i2c_sda_config_in();
    for(index=0; index<8; index++)
    {
        i2c_delay(I2C_DELAY_COUNT);
        I2C_SCL_H();
        value |= I2C_SDA_INPUT();
        if(index<7)
        {
            value <<= 1;
        }
        i2c_delay(I2C_DELAY_COUNT);
        I2C_SCL_L();
    }
    
    if(!ack_status)
        i2c_send_nack();
    else
        i2c_send_ack();
    
    return value;
}

uint8_t i2c_wait_ack(void)
{
    uint16_t wait_time = 0;

    i2c_sda_config_in();
    i2c_delay(I2C_DELAY_COUNT);

    while (I2C_SDA_INPUT())
    {
        wait_time++;
        if(wait_time > 10000)
        {
            return 1;
        }
    }
    
    //master read ack.
    I2C_SCL_H();
    i2c_delay(I2C_DELAY_COUNT);
    I2C_SCL_L();   
    return 0;
}

void i2c_stop(void)
{
    i2c_sda_config_out();
    I2C_SDA_L();
    i2c_delay(I2C_DELAY_COUNT);	
	I2C_SCL_H();
    i2c_delay(I2C_DELAY_COUNT);
	I2C_SDA_H();  
    i2c_delay(I2C_DELAY_COUNT);
	I2C_SCL_L(); 
}

uint8_t i2c_write_device(uint8_t addr, uint8_t data)
{
    //1. send start
    i2c_start();

    //2. send the address
    i2c_send_byte(addr&0xfe);  //the bit0 is w, must 0
    if(i2c_wait_ack())
    {
        i2c_stop();
        return 1;
    }

    //3. send the data
    i2c_send_byte(data);
    if(i2c_wait_ack())
    {
        i2c_stop();
        return 2;
    }

    //4. send the stop
    i2c_stop();

    i2c_delay(I2C_DELAY_COUNT);
    return 0;     
}

uint8_t i2c_read_device(uint8_t addr, uint8_t *rdata)
{
    uint8_t data;

    //1. send start
    i2c_start();

    //2. send the address
    i2c_send_byte(addr|0x01);  //the bit0 is r, must 1
    if(i2c_wait_ack())
    {
        i2c_stop();
        return 1;
    }

    //3. read the data
    data = i2c_read_byte(0);

    //4. send the stop
    i2c_stop();

    *rdata = data;
    
    return 0;     
}