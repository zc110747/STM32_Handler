//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      rng.h
//
//  Purpose:
//      rng driver interface.
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
uint32_t calc_hw_crc32(uint32_t *pbuffer, uint32_t size);

uint8_t calc_crc8(uint8_t *ptr, uint32_t len);    
uint16_t calc_crc16(uint8_t *ptr, uint16_t len);    
uint32_t calc_crc32(uint32_t *data, size_t length);
#ifdef __cplusplus
}
#endif
#endif
