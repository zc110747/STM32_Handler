/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

//c interface
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
    
//freertos header
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "message_buffer.h"
#include "product_config.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
typedef enum
{
    GLOBAL_OK = 0,
    GLOBAL_ERROR = 1,
}GlobalType_t;

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define OUTPUT_BEEP                 0
#define INPUT_AP_INT                1
#define OUTPUT_DCMI_PWDN            2
#define OUTPUT_USB_PWR              3
#define INPUT_EXIO                  4
#define INPUT_D_INT                 5
#define OUTPUT_RS485_SEL            6
#define OUTPUT_ETH_RESET            7

#define IO_OFF                      0
#define IO_ON                       1

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
