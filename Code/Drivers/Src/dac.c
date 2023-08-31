//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      dac.c
//
//  Purpose:
//      dac init and set_voltage.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "dac.h"

static DAC_HandleTypeDef dac_handle_;

static BaseType_t dac_test();
static BaseType_t dac_hardware_init();
    
BaseType_t dac_init()
{
    BaseType_t result;

    result = dac_hardware_init();

    if(result == pdPASS)
    {
        dac_set_voltage(DAC_DEFAULT_VOL);

        dac_test();
    }
    else
    {
        printf("dac_driver hardware_init failed\r\n");
    }
    return result;  
}

void dac_set_voltage(uint16_t mv)
{
    float adc_value;
    
    if(mv > DAC_REFERENCE_VOL)
        mv = DAC_REFERENCE_VOL;
    
    adc_value = (float)mv/DAC_REFERENCE_VOL * DAC_MAX_VALUE;
    
    HAL_DAC_Stop(&dac_handle_, DAC_CHANNEL_1);
    HAL_DAC_SetValue(&dac_handle_, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint32_t)adc_value);
    HAL_DAC_Start(&dac_handle_, DAC_CHANNEL_1);    
}
    
static BaseType_t dac_hardware_init()
{
    DAC_ChannelConfTypeDef sConfig = {0};

    dac_handle_.Instance = DAC;
    if(HAL_DAC_Init(&dac_handle_) != HAL_OK)
        return pdFAIL;

    sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    if(HAL_DAC_ConfigChannel(&dac_handle_, &sConfig, DAC_CHANNEL_1) != HAL_OK)
        return pdFAIL;
    
    return pdPASS;
}

static BaseType_t dac_test()
{
    return pdPASS;
}
