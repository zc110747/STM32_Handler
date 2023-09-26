//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_alg.c
//
//  Purpose:
//      algorithm driver(rng, crc)
//      stm32f1, stm32f4的crc只支持crc32, 且输入必须为4字节, 实际使用需要注意
//      传入指针的对齐和长度需要补足,使用容易出错，适用范围也小, 不建议使用, 
//      计算法或者查表法适合使用。
//      影响CRC输出的有四方面, 多项式, 初始值, 高/低位优先计算, 输出反向
//      stm32f4的hal hash库内定义缺失,先不考虑了解
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_alg.h"

static RNG_HandleTypeDef hrng;
static CRC_HandleTypeDef hcrc;

static BaseType_t rng_init(void);
static BaseType_t crc_init(void);

BaseType_t alg_driver_init(void)
{
    BaseType_t result = pdPASS;
    
    //rng init
    result &= rng_init();
    
    //crc init
    result &= crc_init();
    
    return result;
}

//rng application
static BaseType_t rng_init(void)
{
    __HAL_RCC_RNG_CLK_ENABLE();
    
    hrng.Instance = RNG;
    
    if (HAL_RNG_Init(&hrng) != HAL_OK)
        return pdFAIL;
   
    return pdPASS;    
}

uint32_t rng_get_value(void)
{
    uint32_t value = 0;
    
    HAL_RNG_GenerateRandomNumber(&hrng, &value);
    
    return value;
}

static BaseType_t crc_init(void)
{
    __HAL_RCC_CRC_CLK_ENABLE();
    
    hcrc.Instance = CRC;
    if (HAL_CRC_Init(&hcrc) != HAL_OK)
    {
        return pdFAIL;
    }
    
    return pdPASS;
}

uint32_t calc_hw_crc32(uint32_t *pbuffer, uint32_t size)
{
    uint32_t value;
    
    value = HAL_CRC_Calculate(&hcrc, pbuffer, size);
    
    return value;
}

#define CRC8_POLYNOMIAL    0x07
#define CRC8_INIT          0xFF
uint8_t calc_crc8(uint8_t *ptr, uint32_t len)
{
	unsigned int i;
	unsigned char crc = CRC8_INIT;
	
	while(len--)
	{
        crc ^= *ptr++;
        for ( i = 0; i < 8; i++ )
        {
            if ( crc & 0x80 )
                crc = (crc << 1)^CRC8_POLYNOMIAL;
            else
                crc <<= 1;
        }
	}
	
	return crc;
}

#define CRC16_POLYNOMIAL    0xA001
#define CRC16_INIT          0xFFFF
uint16_t calc_crc16(uint8_t *ptr, uint16_t len)
{
    uint8_t i;
    uint16_t crc = CRC16_INIT;
    while(len--)
    {
        crc ^= *ptr++;            
        for (i = 0; i < 8; ++i)
        {
            if (crc & 0x80)
                crc = (crc << 1)^CRC16_POLYNOMIAL;        
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

#define CRC32_POLYNOMIAL    0x04C11DB7
#define CRC32_INIT          0xFFFFFFFF
uint32_t calc_crc32(uint32_t *data, size_t length) 
{
    uint32_t crc = CRC32_INIT;
    for (size_t i = 0; i < length; i++) 
    {
        crc ^= data[i];
        for (size_t j = 0; j < 32; j++) 
        {
            if (crc & 0x80000000)
            {
                crc = (crc<<1)^CRC32_POLYNOMIAL;
            } 
            else 
            {
                crc = (crc<<1);
            }
        }
    }
    return crc;
}
