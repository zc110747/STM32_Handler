//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     drv_timer.c
//
//  Purpose:
//     timer driver.
//
// Author:
//     @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_timer.h"

static TIM_HandleTypeDef htim5;

//硬件连接PB1和PH11即可读取一段时间的计数值
#if TIME_EXTEND_ENCODE == 0
BaseType_t timer_extend_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    __HAL_RCC_TIM5_CLK_ENABLE();

    __HAL_RCC_GPIOH_CLK_ENABLE();

    /**TIM5 GPIO Configuration
    PH11     ------> TIM5_CH2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM5_Init 1 */

    /* USER CODE END TIM5_Init 1 */
    htim5.Instance = TIM5;
    htim5.Init.Prescaler = 0;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 4294967295;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
    {
        return pdFAIL;
    }

    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
    sSlaveConfig.InputTrigger = TIM_TS_TI2FP2;
    sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
    sSlaveConfig.TriggerFilter = 0;
    if (HAL_TIM_SlaveConfigSynchro(&htim5, &sSlaveConfig) != HAL_OK)
    {
        return pdFAIL;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
    {
        return pdFAIL;
    }
    
    __HAL_TIM_SetCounter(&htim5, 0);
    HAL_TIM_Base_Start(&htim5);
    return pdPASS;
}

uint32_t get_cnt_per_second(void)
{
    uint32_t cnt;
    
    cnt = __HAL_TIM_GetCounter(&htim5);
    __HAL_TIM_SetCounter(&htim5, 0);
    
    return cnt;
}
#else


#endif