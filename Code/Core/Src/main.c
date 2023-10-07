//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      main.c
//
//  Purpose:
//
// Author:
//      @zc
//
//  Assumptions:
//	
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "driver.hpp"
#include "application.hpp"
#include "cm_backtrace.h"
#include "lwip.h"
#include "dsp.h"

////local define

////global parameter
SRAM_HandleTypeDef hsram1;

///local function
static BaseType_t SystemClock_Config(void);

int main(void)
{
    BaseType_t xReturned;
      
    HAL_Init();

    SystemClock_Config();
    
    //logger interface init, init before use PRINT_LOG.
    xReturned = logger_init();
    
    //uart driver init for logger
    xReturned &= usart_driver_init();
    
    //logger interface must start, otherwise printf not work.
    //cm_backtrace_init("STM32 Handler", TRACE_HARDWARE_VERSION, TRACE_SOFTWARE_VERSION);  
    
    //driver interface init
    xReturned &= driver_init();
    
    //dfu&dsp test
    xReturned &= dsp_app();
    
    //aplication init, must after driver init because call driver
    xReturned &= application_init();
    
    //start the rtos schedular.
    vTaskStartScheduler();
   
    //if run to this, start failed
    printf("system start failed, please check!\r\n");
    
    while (1)
    {

    }
}

//task idle run, can process rx command
void vApplicationIdleHook(void)
{
    logger_process_run();
}

static BaseType_t SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    //SYSCLK 25M/25*360/2=180M
    //AHB SYSCLK/1=180M
    //APB1 AHB/4=45M 
    //APB1-TIME APB1*2=90M 
    //APB2 AHB/2 = 90M
    //APB2-TIME APB2*2=180M
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                          |RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;    //Bypass/Crystal, according the hardware
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 360;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        return pdFAIL;
    }

    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        return pdFAIL;
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                            |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        return pdFAIL;
    }
    return pdPASS;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
