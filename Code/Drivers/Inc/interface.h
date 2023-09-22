

#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "main.h"
#include "application.hpp"
#include "logger_process.h"

typedef enum
{
    LED0 = 0,
}led_device;

//led status
typedef enum
{
    LED_STATUS_OFF = 0,
    LED_STATUS_ON,
}led_status;

#ifdef __cplusplus
extern "C" {
#endif
void set_os_on(void);

//if us, use loop delay, if max than 1000, use delay ms.
void delay_us(uint16_t us);
void delay_ms(uint16_t ms);
#ifdef __cplusplus
}
#endif
#endif