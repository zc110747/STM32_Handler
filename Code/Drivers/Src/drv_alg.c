//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     drv)alg.c
//
//  Purpose:
//     algorithm driver(rng, crc)
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

uint32_t crc_get_value(uint32_t *pbuffer, uint32_t size)
{
    uint32_t value;
    
    value = HAL_CRC_Calculate(&hcrc, pbuffer, size);
    
    return value;
}