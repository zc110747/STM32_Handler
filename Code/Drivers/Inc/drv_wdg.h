//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_wdg.h
//
//  Purpose:
//      watchdog application.
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

void iwdg_reload(void);
BaseType_t wdg_driver_init(void);    
    
#ifdef __cplusplus
}
#endif 
