//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      usart.h
//
//  Purpose:
//      usart driver interface process.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DRV_USART_H
#define _DRV_USART_H

#include "interface.h"

#define USART_MODE_POLL             0
#define USART_MODE_INTERRUPT        1
#define USART_MODE_DMA              2

#define USART_RUN_MODE              USART_MODE_INTERRUPT

#define USART_TRANSLATE_DELAY_TIME  (200)

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t usart_driver_init(void);
void usart_translate(char *ptr, uint16_t size);
void usart_receive(char *ptr, uint16_t size);
    
#ifdef __cplusplus
}
#endif
#endif
