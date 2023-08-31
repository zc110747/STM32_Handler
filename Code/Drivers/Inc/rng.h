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
_Pragma("once");

#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif
    
BaseType_t rng_init();
uint32_t rng_get_value(void);
    
#ifdef __cplusplus
}
#endif