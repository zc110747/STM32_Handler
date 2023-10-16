//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_led.h
//
//  Purpose:
//      led driver.
//      hardware: 
//          LED0 ------------ PB0
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _LED_H
#define _LED_H

#include "interface.h"

#define LED0_ON     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
#define LED0_OFF    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

#ifdef __cplusplus
    extern "C" {
#endif
        
BaseType_t led_driver_init(void);
#ifdef __cplusplus
}
#endif
#endif
