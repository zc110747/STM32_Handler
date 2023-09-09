
#include "drv_pwm.h"

#define TIME_PWM_PEROID 1000

static TIM_HandleTypeDef htim3_pwm;

BaseType_t pwm_driver_init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    //STM32F42x�����ֲ������ŵĸ���˵��
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //����TIM3��ʱ��
    htim3_pwm.Instance = TIM3;
    htim3_pwm.Init.Prescaler = 89;                          //��Ƶ������ʱ��ΪTimerClk/(prescaler+1), ��90MΪ����Ϊ1M
    htim3_pwm.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3_pwm.Init.Period = TIME_PWM_PEROID;                //�Զ���װ�ؼ�����ֵ
    htim3_pwm.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;  //��Ƶ��ΪTimer��ʵ������ʱ��
    htim3_pwm.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&htim3_pwm) != HAL_OK)
    {
        return pdFAIL;
    }
    
    //����timerѡ���ʱ����Դ
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3_pwm, &sClockSourceConfig) != HAL_OK)
    {
        return pdFAIL;
    }

    //�ر��ⲿ����
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3_pwm, &sMasterConfig) != HAL_OK)
    {
        return pdFAIL;
    }

    //����Output Compare���ܣ�����PWM���
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = TIME_PWM_PEROID/2;  //50%ռ�ձ�
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