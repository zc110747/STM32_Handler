//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     drv_pwm.c
//
//  Purpose:
//     timer pwm output driver.
//
// Author:
//     @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_pwm.h"

#define TIME_PWM_PEROID 1000

static TIM_HandleTypeDef htim3_pwm;

#if PWM_RUN_MODE == PWM_RUN_NORMAL
BaseType_t pwm_driver_init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    //STM32F42x数据手册有引脚的复用说明
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //配置TIM3的时钟
    htim3_pwm.Instance = TIM3;
    htim3_pwm.Init.Prescaler = 89;                          //分频，计数时钟为TimerClk/(prescaler+1), 以90M为例则为1M
    htim3_pwm.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3_pwm.Init.Period = TIME_PWM_PEROID;                //自动重装载计数器值
    htim3_pwm.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;  //分频作为Timer的实际输入时钟
    htim3_pwm.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&htim3_pwm) != HAL_OK)
    {
        return pdFAIL;
    }
    
    //定义timer选择的时钟来源
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3_pwm, &sClockSourceConfig) != HAL_OK)
    {
        return pdFAIL;
    }

    //关闭外部触发
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3_pwm, &sMasterConfig) != HAL_OK)
    {
        return pdFAIL;
    }

    //配置Output Compare功能，用于PWM输出
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = TIME_PWM_PEROID/2;  //50%占空比
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim3_pwm, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
    {
        return pdFAIL;
    }
    HAL_TIM_PWM_Start(&htim3_pwm, TIM_CHANNEL_4);
    return pdPASS;
}

//modify the percent for square wave
void pwm_set_percent(float percent)
{
    uint16_t period = TIME_PWM_PEROID*percent;
    
    htim3_pwm.Instance->CCR4 = period;
}
#else

uint16_t timer_pwm_buffer[] = {
    200, 400, 600, 800
};
static DMA_HandleTypeDef hdma_tim3;
BaseType_t pwm_driver_init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    //STM32F42x数据手册有引脚的复用说明
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //配置TIM3的时钟
    htim3_pwm.Instance = TIM3;
    htim3_pwm.Init.Prescaler = 89;                          //分频，计数时钟为TimerClk/(prescaler+1), 以90M为例则为1M
    htim3_pwm.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3_pwm.Init.Period = TIME_PWM_PEROID;                //自动重装载计数器值
    htim3_pwm.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;  //分频作为Timer的实际输入时钟
    htim3_pwm.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&htim3_pwm) != HAL_OK)
    {
        return pdFAIL;
    }
    
    //定义timer选择的时钟来源
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3_pwm, &sClockSourceConfig) != HAL_OK)
    {
        return pdFAIL;
    }

    //关闭外部触发
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3_pwm, &sMasterConfig) != HAL_OK)
    {
        return pdFAIL;
    }

    //配置Output Compare功能，用于PWM输出
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = TIME_PWM_PEROID/2;  //50%占空比
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim3_pwm, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
    {
        return pdFAIL;
    }
    
    //TIM3_CH4, TIM3_UPDATA对应的DMA通道为
    //DMA1_Stream2, Channel5, 详细见参考手册10.3.3 Channel Selection
    hdma_tim3.Instance = DMA1_Stream2;
    hdma_tim3.Init.Channel = DMA_CHANNEL_5;
    hdma_tim3.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim3.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim3.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim3.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tim3.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_tim3.Init.Mode = DMA_CIRCULAR;
    hdma_tim3.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_tim3.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_tim3) != HAL_OK)
    {
      return pdFAIL;
    }
    
    //使用UPDATA作为PWM更换周期
    __HAL_TIM_ENABLE_DMA(&htim3_pwm, TIM_DMA_UPDATE);
    HAL_DMA_Start(&hdma_tim3, (uint32_t)timer_pwm_buffer, (uint32_t)&htim3_pwm.Instance->CCR4, 4);
    HAL_TIM_PWM_Start(&htim3_pwm, TIM_CHANNEL_4);
    return pdPASS;
}

//modify the percent for square wave
void pwm_set_percent(float percent)
{
    uint8_t index;
    
    for(index=0; index<4; index++)
    {
        timer_pwm_buffer[index] = timer_pwm_buffer[index]*percent;
    }
}

#endif
