//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      interface.c
//
//  Purpose:
//      interface used for system and library.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "interface.h"

uint8_t is_os_on = 0;

void set_os_on()
{
    is_os_on = 1;
}

void delay_ms(uint16_t ms)
{
    if(is_os_on)
    {
        vTaskDelay(ms);
    }
    else
    {
        HAL_Delay(ms);
    }
}

void delay_us(uint16_t times)
{
    uint16_t i, j;
    for(i=0; i<times; i++)
    {
        for(j=0; j<5; j++)
        {
            __NOP();
        }
    }
}
