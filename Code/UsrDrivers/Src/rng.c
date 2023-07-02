//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     rng.c
//
//  Purpose:
//     rng driver.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "rng.h"

static RNG_HandleTypeDef rng_handler_;
    
BaseType_t rng_init()
{
    rng_handler_.Instance = RNG;
    if (HAL_RNG_Init(&rng_handler_) != HAL_OK)
    {
        return pdFAIL;
    }
    return pdPASS;    
}

uint32_t rng_get_value(void)
{
    uint32_t value = 0;
    
    HAL_RNG_GenerateRandomNumber(&rng_handler_, &value);
    
    return value;
}
