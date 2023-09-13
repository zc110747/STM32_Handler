//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     tpad.hpp
//
//  Purpose:
//     tpad key driver interface.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef DRV_TPAD_H
#define DRV_TPAD_H

#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t tpad_scan_key(void);
BaseType_t tpad_driver_init(void);
uint16_t tpad_get_no_push_val(void);
uint16_t tpad_current_val(void);

#ifdef __cplusplus
}
#endif
#endif
