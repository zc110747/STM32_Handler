#ifndef _DRV_SOFT_I2C_H
#define _DRV_SOFT_I2C_H

#include "main.h"

#define I2C_SCL_PIN     GPIO_PIN_4
#define I2C_SCL_PORT    GPIOH
#define I2C_SDA_PIN     GPIO_PIN_5
#define I2C_SDA_PORT    GPIOH

#define I2C_SCL_H()     HAL_GPIO_WritePin(I2C_SCL_PORT, I2C_SCL_PIN, GPIO_PIN_SET)
#define I2C_SCL_L()     HAL_GPIO_WritePin(I2C_SCL_PORT, I2C_SCL_PIN, GPIO_PIN_RESET)
#define I2C_SDA_H()     HAL_GPIO_WritePin(I2C_SDA_PORT, I2C_SDA_PIN, GPIO_PIN_SET)
#define I2C_SDA_L()     HAL_GPIO_WritePin(I2C_SDA_PORT, I2C_SDA_PIN, GPIO_PIN_RESET)
#define I2C_SDA_INPUT() (HAL_GPIO_ReadPin(I2C_SDA_PORT, I2C_SDA_PIN) == GPIO_PIN_SET)

uint8_t i2c_write_device(uint8_t addr, uint8_t data);
uint8_t i2c_read_device(uint8_t addr, uint8_t *rdata);

#endif