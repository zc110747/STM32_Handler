//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      cmd_process.h
//
//  Purpose:
//     
//
// Author:
//      @zc
//
// Revision History:
//      Version V1.0b2 Create.
/////////////////////////////////////////////////////////////////////////////
#ifndef __CMD_PROCESS_H
#define __CMD_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include "main.h"

#define COMMAND_MAX_SIZE    32
typedef struct  
{
  uint8_t index;
  uint8_t rx_buffer[COMMAND_MAX_SIZE+1];
}LOGGER_COMMAND_BUFFER;

void cmd_protocol_init(void);
uint8_t cmd_process(uint8_t c);

#ifdef __cplusplus
}
#endif

#endif
