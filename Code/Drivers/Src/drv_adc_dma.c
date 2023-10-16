//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      adc.cpp
//
//  Purpose:
//      adc driver normal get.
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

#if ADC_RUN_MODE == ADC_RUN_DMA
#define ADC_BUFFER_SIZE     ADC_AVG_TIMES*2

uint16_t ADC_Buffer[ADC_BUFFER_SIZE];

static ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
static BaseType_t adc_calibration_init(void);
uint32_t vref_val = 3300;

BaseType_t adc_driver_init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV6;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = ENABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 2;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        return pdFAIL;
    }

    sConfig.Channel = ADC_CHANNEL_6;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        return pdFAIL;
    }

    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = 2;
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        return pdFAIL;
    }
    
    //run adc calibration
    adc_calibration_init();
    
    //enable the adc dma
    SET_BIT(hadc1.Instance->CR2, ADC_CR2_DMA);
  
    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
        return pdFAIL;
    }
    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);
    
    __HAL_DMA_ENABLE_IT(&hdma_adc1, DMA_IT_TC);
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    
    HAL_DMA_Start(&hdma_adc1, (uint32_t)&hadc1.Instance->DR, (uint32_t)ADC_Buffer, ADC_BUFFER_SIZE);
    HAL_ADC_Start(&hadc1);
    
    return pdPASS;
}

void DMA2_Stream0_IRQHandler(void)
{
    if(__HAL_DMA_GET_FLAG(&hdma_adc1, DMA_FLAG_TCIF0_4) != RESET)
    {      
       __HAL_DMA_CLEAR_FLAG(&hdma_adc1, DMA_FLAG_TCIF0_4);
       __HAL_DMA_CLEAR_FLAG(&hdma_adc1, DMA_FLAG_TEIF0_4);
    }
}

uint32_t get_vref_val(void)
{
    return vref_val;
}

static BaseType_t adc_calibration_init(void)
{
    ADC_InjectionConfTypeDef sConfigInjected = {0};    

    sConfigInjected.InjectedChannel = ADC_CHANNEL_VREFINT;
    sConfigInjected.InjectedRank = 1;
    sConfigInjected.InjectedNbrOfConversion = 1;
    sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_56CYCLES;
    sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONVEDGE_NONE;
    sConfigInjected.ExternalTrigInjecConv = ADC_INJECTED_SOFTWARE_START;
    sConfigInjected.AutoInjectedConv = DISABLE;
    sConfigInjected.InjectedDiscontinuousConvMode = ENABLE;
    sConfigInjected.InjectedOffset = 0;
    if (HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected) != HAL_OK)
    {
        return pdFAIL;
    }

    HAL_ADCEx_InjectedStart(&hadc1);
    
    if(HAL_ADCEx_InjectedPollForConversion(&hadc1, 100) != HAL_OK)
    {
        return pdFAIL;
    }

    vref_val = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
    vref_val = __LL_ADC_CALC_VREFANALOG_VOLTAGE(vref_val, LL_ADC_RESOLUTION_12B);
    
    PRINT_LOG(LOG_INFO, "ADC Calcbration success, vref:%d", vref_val);
    
    return pdPASS;
}

uint16_t adc_avg_value_read(uint32_t channel)
{
    uint32_t temp = 0;
    uint8_t index;
    uint16_t *pstart;
    
    switch(channel)
    {
        case ADC_CHANNEL_6:
            pstart = ADC_Buffer;
            break;
        case ADC_CHANNEL_TEMPSENSOR:
            pstart = ADC_Buffer+1;
            break;
        default:
           pstart = ADC_Buffer;
            break;   
    }
    
    for(index=0; index<ADC_AVG_TIMES; index++)
    {
        temp += pstart[index*2];
    }
    
    return temp/ADC_AVG_TIMES;
}

#endif

