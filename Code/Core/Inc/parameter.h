//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      parameter.h
//
//  Purpose:
//      internal parameter process, saved in flash, used
//
// Author:
//      @zc
//
//  Assumptions:
//	
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _PARAMETER_H
#define _PARAMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "logger_process.h"

typedef struct
{
    //log interface
    LOG_DEVICE log_dev;
    LOG_LEVEL log_lev;

    //log uart config
    uint32_t log_Baud;
    uint32_t log_DataBit;
    uint32_t log_StopBit;
    uint32_t log_Parity;
    
    //log ip port(server)
    uint16_t log_port;
    
    //bake for future
    uint8_t  reserved[16];
}LOGGER_PARAMETER;

typedef struct
{
    //internet
    uint8_t   is_dhcp;
    uint8_t   ipaddress[4];
    uint8_t   netmask[4];
    uint8_t   gateway[4];
   
    //support web and software(server)
    uint16_t  http_port;
    uint16_t  socket_port;

    //bake for future
    uint8_t  reserved[16];    
}NET_PARAMETER;

typedef struct
{
    LOGGER_PARAMETER log_info;
    
    NET_PARAMETER   net_info;
}GLOBAL_PARAMETER;

#ifdef __cplusplus
}
#endif

#endif
