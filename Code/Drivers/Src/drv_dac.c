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
#include "drv_dac.h"

//global parameter
static DAC_HandleTypeDef hdac;
static DMA_HandleTypeDef hdma_dac1;
static TIM_HandleTypeDef htim;

//global function
static void dac_run_test(void);

void dac_voltage_write(uint16_t mv)
{
    float adc_value;
    
    if(mv > DAC_REFERENCE_VOL)
        mv = DAC_REFERENCE_VOL;
    
    adc_value = (float)mv/DAC_REFERENCE_VOL * DAC_MAX_VALUE;
    
    HAL_DAC_Stop(&hdac, DAC_CHANNEL_1);
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint32_t)adc_value);
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);    
}


#if DMA_RUN_MODE == DMA_MODE_POLL
BaseType_t dac_driver_init()
{
    DAC_ChannelConfTypeDef sConfig = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_DAC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    hdac.Instance = DAC;
    if(HAL_DAC_Init(&hdac) != HAL_OK)
        return pdFAIL;

    sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    if(HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
        return pdFAIL;
     
    dac_voltage_write(2000);   
    
#if RUN_TEST_MODE == DAC_TEST
    dac_run_test();
#endif    
    return pdPASS; 
}

static void dac_run_test(void)
{
    dac_voltage_write(1000);
    
    while(1)
    {
    
    }
}

void dac_convert_voltage_write(float percent)
{
    uint32_t adc_vol;
    
    adc_vol = DAC_MAX_VALUE*percent;
    dac_voltage_write(adc_vol);
}
#else

uint16_t vol_cycle[] = {
    2048, 2368, 2680, 2977, 3251,
    3496, 3704, 3872, 3995, 4070,
    4095, 4070, 3995, 3872, 3704,
    3496, 3251, 2977, 2680, 2368,
    2048, 1727, 1415, 1118, 844,
    599,  391,  223,  100,  25,
    0,    25,   100,  223,  391,
    599,  844, 1118, 1415, 1727,
};
uint16_t vol_convert_cycle[40];

void dac_convert_voltage_write(float percent)
{
    uint8_t i;
    
    for(i=0; i<40; i++)
    {
        vol_convert_cycle[i] = percent*vol_cycle[i];
    }
}

BaseType_t dac_driver_init()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    DAC_ChannelConfTypeDef sConfig = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_DAC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    hdac.Instance = DAC;
    if(HAL_DAC_Init(&hdac) != HAL_OK)
        return pdFAIL;

    sConfig.DAC_Trigger = DAC_TRIGGER_T4_TRGO;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
    if(HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
        return pdFAIL;
    
    /* DAC DMA Init */
    hdma_dac1.Instance = DMA1_Stream5;
    hdma_dac1.Init.Channel = DMA_CHANNEL_7;
    hdma_dac1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_dac1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dac1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_dac1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_dac1.Init.Mode = DMA_CIRCULAR;
    hdma_dac1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_dac1.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_dac1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
    hdma_dac1.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_dac1.Init.PeriphBurst = DMA_MBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_dac1) != HAL_OK)
    {
      return pdFAIL;
    }
    __HAL_LINKDMA(&hdac, DMA_Handle1, hdma_dac1);
    
    //initialize timer
    //clock APB1 90M
    htim.Instance = TIM4;
    htim.Init.Prescaler = 89; //1M
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.Period = 1;
    htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim) != HAL_OK)
    {
      return pdFAIL;
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim, &sClockSourceConfig) != HAL_OK)
    {
      return pdFAIL;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig) != HAL_OK)
    {
      return pdFAIL;
    }
  
    dac_convert_voltage_write(1.0);
    
    //enable dac and dma send
    HAL_TIM_Base_Start(&htim);
    SET_BIT(hdac.Instance->CR, DAC_CR_DMAEN1);
    HAL_DMA_Start(&hdma_dac1, (uint32_t)vol_convert_cycle, (uint32_t)&hdac.Instance->DHR12R1, 40);
    __HAL_DAC_ENABLE(&hdac, DAC_CHANNEL_1);
    
    //HAL_DAC_Start_DMA(&hdac ,DAC_CHANNEL_1,(uint32_t *)vol_cycle, 12, DAC_ALIGN_12B_R);
#if RUN_TEST_MODE == DAC_TEST
    dac_run_test();
#endif    
    return pdPASS; 
}

static void dac_run_test(void)
{
    while(1)
    {
    
    }
}
#endif

