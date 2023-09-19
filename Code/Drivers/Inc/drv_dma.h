//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      dma.c
//
//  Purpose:
//      dma application.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DRV_DMA_H
#define _DRV_DMA_H

#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t dma_driver_init(void);    
BaseType_t dma_check_finish(uint32_t timeout_ms);
    
#ifdef __cplusplus
}
#endif 
#endif
