//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     drv_adc.c
//
//  Purpose:
//     adc driver normal get.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_adc.h"

#if ADC_RUN_MODE == ADC_RUN_NORMAL
static ADC_HandleTypeDef hadc1;

BaseType_t adc_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    //enable the clock
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    //PA6 - ADC1 CHANNEL6
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV6;   
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;          
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;           
    hadc1.Init.ScanConvMode = DISABLE;                      
    hadc1.Init.EOCSelection = DISABLE;                     
    hadc1.Init.ContinuousConvMode = DISABLE;              
    hadc1.Init.NbrOfConversion = 1;                        
    hadc1.Init.DiscontinuousConvMode = DISABLE;             
    hadc1.Init.NbrOfDiscConversion = 0;                    
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;     
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;             

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
        return pdFAIL;
    
    return pdPASS;    
}

uint16_t adc_get_value(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    sConfig.Offset = 0;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    //start the adc run
    HAL_ADC_Start(&hadc1);                              
    HAL_ADC_PollForConversion(&hadc1, 10);     
    
    return HAL_ADC_GetValue(&hadc1);    
}

uint16_t adc_get_avg(uint32_t channel)
{
    uint32_t temp = 0;
    uint8_t index;
    
    for(index=0; index<ADC_AVG_TIMES; index++)
    {
        temp += adc_get_value(channel);
        vTaskDelay(1);
    }
    
    return temp/ADC_AVG_TIMES;
}

#endif