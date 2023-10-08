//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      httpd.h
//
//  Purpose:
//      sample http server, support static and dynamic process.
//      1.not support websocket, will close after http run.
//      2.not support data after the url.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __HTTPD_H
#define __HTTPD_H

#include "main.h"

#define HTTP_PORT           3000
#define RX_BUFFER_MAX_LEN   1600

typedef struct
{
    uint8_t full_package;
    
    //rx buffer info
    uint8_t rx_length;
    uint8_t rx_buffer[RX_BUFFER_MAX_LEN];
}HttpServer_t;

#endif
