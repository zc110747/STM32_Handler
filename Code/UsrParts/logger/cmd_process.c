//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.   
//  All Rights Reserved
//
//  Name:
//      cmd_process.c
//
//  Purpose:
//     
//
// Author:
//      @zc
//
// Revision History:
//      
/////////////////////////////////////////////////////////////////////////////
#include "cmd_process.h"
#include "logger_process.h"

#define CMD_SEARCH_NULL 0xFF

typedef uint16_t (*func_ptr)(char *, uint8_t);
typedef struct 
{
    char *command;
    
    uint8_t comsize;

    func_ptr comfunc;
}CMD_COMMAND_STRUCT;


static uint16_t cmd_show_all(char *pdata, uint8_t len)
{
    PRINT_LOG(LOG_INFO, "show all");
    return len;
}
static uint16_t cmd_help(char *pdata, uint8_t len)
{
    return len;
}
static CMD_COMMAND_STRUCT cmd_struct_list[] = {
  {"!showall",  0, cmd_show_all},
  {"!help",     0, cmd_help},    
  {"!?",        0, cmd_help},  
};

////////////////////////////////////////////////////////////////////////////////////////////
//commad core region, no change
/////////////////////////////////////////////////////////////////////////////////////////////
static void cmd_protocol_process(char *pbuffer, uint8_t len);
static uint8_t search_cmd_list(char *pbuffer, uint8_t cmd_size);
static uint8_t cmd_struct_size = sizeof(cmd_struct_list)/sizeof(CMD_COMMAND_STRUCT);
static LOGGER_COMMAND_BUFFER LoggerBuffer;

void cmd_protocol_init(void)
{
    uint8_t index;
    
    memset((char *)&LoggerBuffer, 0, sizeof(LOGGER_COMMAND_BUFFER));
    for(index=0; index<cmd_struct_size; index++)
    {
        cmd_struct_list[index].comsize = strlen(cmd_struct_list[index].command);
    }
}

uint8_t cmd_process(uint8_t c)
{
    if(c != '\r' && c != '\n')
    {
        LoggerBuffer.rx_buffer[LoggerBuffer.index++] = c;
    }

    if(c == '\n' || LoggerBuffer.index >= COMMAND_MAX_SIZE)
    {
        LoggerBuffer.rx_buffer[LoggerBuffer.index] = '\0';
        cmd_protocol_process((char *)LoggerBuffer.rx_buffer, LoggerBuffer.index);
        LoggerBuffer.index = 0;
    }
    
    return 0;
}

static void cmd_protocol_process(char *pbuffer, uint8_t len)
{
    char* ptr, *pdata;
    uint8_t headsize, datasize, index;
   
    ptr = strchr(pbuffer, ' ');
    if(ptr == NULL)
    {
        headsize = len;
        pdata = NULL;
        datasize = 0;
    }
    else
    {
        headsize = (uint32_t)ptr - (uint32_t)pbuffer;
        pdata = ptr + 1;
        datasize = len - headsize - 1; 
    }
    
    index = search_cmd_list(pbuffer, headsize);
    if(index < cmd_struct_size)
    {
        cmd_struct_list[index].comfunc(pdata, datasize);
    }
    else
    {
        PRINT_LOG(LOG_ERROR, "cmd search failed:%d!", index);  
    }
}    

static uint8_t search_cmd_list(char *pbuffer, uint8_t cmd_size)
{
    uint8_t index;
    
    for(index=0; index<cmd_struct_size; index++)
    {
        if(cmd_size == cmd_struct_list[index].comsize)
        {
            if(strncmp(pbuffer, cmd_struct_list[index].command, cmd_size) == 0)
            {
                break;
            }
        }
    }
    
    if(index >= cmd_struct_size)
        return CMD_SEARCH_NULL;
    
    return index;
}
