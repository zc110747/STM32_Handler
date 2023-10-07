//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      httpd.c
//
//  Purpose:
//      sample http server, support static and dynamic process.
//      1.not support websocket, will close after http run.
//      2.not support data after the url.
//      3.support split data mode when multi translate
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "httpd.h"
#include "lwip/api.h"

//local parameter
static HttpServer_t http_server;

//local task
static void http_run_task(void *parameter);
static void http_server_engine(uint8_t *pbuffer, uint16_t size);

BaseType_t http_server_init(void)
{
    BaseType_t xReturn;
    TaskHandle_t task_hanlder;
    
    memset((char *)&http_server, 0, sizeof(HttpServer_t));
    
    xReturn = xTaskCreate(http_run_task, 
                    "http_run_task",
                    HTTPD_TASK_STACK, 
                    ( void * ) NULL,   
                    HTTPD_TASK_PROITY,
                    &task_hanlder);
   
    return xReturn;
}

static void http_run_task(void *parameter)
{
    struct netconn *http_conn;
    struct netconn *accept_conn;
    struct netbuf *inbuf;
    err_t http_err;
    
    //creat a tcp netconn structure
    http_conn = netconn_new(NETCONN_TCP); 

    //bind http port
    netconn_bind(http_conn, NULL, HTTP_PORT);  
    
    //listen the connect
    netconn_listen(http_conn);
    
    while(1)
    {
        http_err = netconn_accept(http_conn, &accept_conn);
        if(http_err == ERR_OK)
        {
            http_server.full_package = 0;
            
            do
            {
                http_err = netconn_recv(accept_conn, &inbuf);
                if(http_err == ERR_OK)
                {
                    struct pbuf *pbuffer = inbuf->p;
                    char *dst_ptr = (char *)http_server.rx_buffer;
                    uint16_t copylen = 0;
                    
                    while((pbuffer != NULL) && (copylen<RX_BUFFER_MAX_LEN-1))
                    {
                        memcpy(dst_ptr, pbuffer->payload, pbuffer->len);
                        copylen += pbuffer->len;
                        dst_ptr += pbuffer->len;
                        pbuffer = pbuffer->next;
                    }
                    http_server.rx_length = copylen;
                    http_server.rx_buffer[copylen] = '\0';
                    
                    //delete the buffer for next read
                    netbuf_delete(inbuf);        
                    
                    //http engine process
                    http_server_engine(http_server.rx_buffer, http_server.rx_length);
                }
            }while(http_server.full_package == 0);
            
            //close the conn
            netconn_close(accept_conn);
            
            //delete current conn
            netconn_delete(accept_conn);
        }
    }
}

static void http_server_engine(uint8_t *pbuffer, uint16_t size)
{
    
}