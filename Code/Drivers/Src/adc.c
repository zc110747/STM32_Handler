//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     adc.cpp
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
#include "adc.h"

static ADC_HandleTypeDef adc1_hander_;
    
static BaseType_t adc_hardware_init(void);

BaseType_t adc_init(void)
{
    BaseType_t result;
    
    result = adc_hardware_init();
    if(result != pdPASS)
    {
        printf("adc hardware_init failed\r\n");
    }
    
    return result;    
}

uint16_t adc_get_value(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
    sConfig.Offset = 0;
    HAL_ADC_ConfigChannel(&adc1_hander_, &sConfig);
    
    //start the adc run
    HAL_ADC_Start(&adc1_hander_);                              
    HAL_ADC_PollForConversion(&adc1_hander_, 10);     
    
    return HAL_ADC_GetValue(&adc1_hander_);    
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


static BaseType_t adc_hardware_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
//    
//    //Enable Hardware analog pin
    //PA6 - ADC1 CHANNEL6
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    adc1_hander_.Instance=ADC1;
    adc1_hander_.Init.ClockPrescaler=ADC_CLOCK_SYNC_PCLK_DIV4;   
    adc1_hander_.Init.Resolution=ADC_RESOLUTION_12B;          
    adc1_hander_.Init.DataAlign=ADC_DATAALIGN_RIGHT;           
    adc1_hander_.Init.ScanConvMode=DISABLE;                      
    adc1_hander_.Init.EOCSelection=DISABLE;                     
    adc1_hander_.Init.ContinuousConvMode=DISABLE;              
    adc1_hander_.Init.NbrOfConversion=1;                        
    adc1_hander_.Init.DiscontinuousConvMode=DISABLE;             
    adc1_hander_.Init.NbrOfDiscConversion=0;                    
    adc1_hander_.Init.ExternalTrigConv=ADC_SOFTWARE_START;     
    adc1_hander_.Init.ExternalTrigConvEdge=ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc1_hander_.Init.DMAContinuousRequests=DISABLE;             

    if (HAL_ADC_Init(&adc1_hander_) != HAL_OK)
        return pdFAIL;

    return pdPASS;    
}
