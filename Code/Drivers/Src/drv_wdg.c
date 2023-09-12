//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     drv_wdg.c
//
//  Purpose:
//     watchdog application.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////

#include "drv_wdg.h"

IWDG_HandleTypeDef hiwdg;
WWDG_HandleTypeDef hwwdg;

static BaseType_t iwdg_init(void);
static BaseType_t wwdg_init(void);

BaseType_t wdg_driver_init(void)
{
    BaseType_t result;
    
    result = iwdg_init();
    
    result &= wwdg_init();
    
    return result;
}

//iwdg clock 32K, count 1K = 1ms
//times 4.095s
static BaseType_t iwdg_init(void)
{
#if IWDG_MODULE_ON == 1
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_32; 
    hiwdg.Init.Reload = 4095;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        return pdFAIL;
    }
#endif
    return pdPASS;
}

void iwdg_reload(void)
{
#if IWDG_MODULE_ON == 1
   HAL_IWDG_Refresh(&hiwdg);
#endif
}

//PCLK1 45M, 45M/4096/8 = 1.373ms
//times = (127-64)*1.373 = 86ms
static BaseType_t wwdg_init(void)
{
#if WWDG_MODULE_ON == 1
    __HAL_RCC_WWDG_CLK_ENABLE();

    HAL_NVIC_SetPriority(WWDG_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(WWDG_IRQn);
    
    hwwdg.Instance = WWDG;
    hwwdg.Init.Prescaler = WWDG_PRESCALER_8;
    hwwdg.Init.Window = 90;
    hwwdg.Init.Counter = 127;
    hwwdg.Init.EWIMode = WWDG_EWI_ENABLE;
    if (HAL_WWDG_Init(&hwwdg) != HAL_OK)
    {
        return pdFAIL;
    }
#endif
 
    return pdPASS;
}

void WWDG_IRQHandler(void)
{
    if (__HAL_WWDG_GET_FLAG(&hwwdg, WWDG_FLAG_EWIF) != RESET)
    {
        __HAL_WWDG_CLEAR_FLAG(&hwwdg, WWDG_FLAG_EWIF);
        HAL_WWDG_Refresh(&hwwdg);
    }
}
