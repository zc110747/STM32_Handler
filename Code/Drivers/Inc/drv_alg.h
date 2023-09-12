//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     rng.h
//
//  Purpose:
//     rng driver interface.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DRV_ALG_H
#define _DRV_ALG_H

#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif
    
BaseType_t alg_driver_init(void);
uint32_t rng_get_value(void);
uint32_t crc_get_value(uint32_t *pbuffer, uint32_t size);
    
#ifdef __cplusplus
}
#endif
#endif
