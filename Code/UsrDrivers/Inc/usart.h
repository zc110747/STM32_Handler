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
_Pragma("once");

#include "interface.h"

#define USART_TRANSLATE_DELAY_TIME  (200)

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t usart_init(void);
BaseType_t usart1_translate(char *ptr, uint16_t size);

#ifdef __cplusplus
}
#endif