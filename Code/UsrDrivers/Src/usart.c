//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      usart.cpp
//          USART_TX -------------- PA9
//          USART_RX -------------- PA10
//
//  Purpose:
//      usart driver interrupt rx and normal tx.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "usart.h"

static UART_HandleTypeDef huart1;
static BaseType_t is_usart_init = pdFALSE;

static BaseType_t usart_test(void);
static BaseType_t usart_hardware_init(void);

BaseType_t usart_init(void)
{
    BaseType_t result;
    
    //device initialize
    result  = usart_hardware_init();
    if(result == pdPASS)
    {
        is_usart_init = pdTRUE;

        usart_test();
    }
    
    return result;
}

BaseType_t usart1_translate(char *ptr, uint16_t size)
{
    BaseType_t result = pdPASS;

    if(!is_usart_init)
    {
        return pdFAIL;
    }

    if(HAL_UART_Transmit(&huart1, (uint8_t *)ptr, size, USART_TRANSLATE_DELAY_TIME) != HAL_OK)
    {
        result = pdFAIL;
    }
    return result;
}
    
static BaseType_t usart_test(void)
{
#if UART_TEST == 1
    usart1_translate((char *)"hello world\r\n", strlen("hello world\r\n"));
    
    printf("This is for hello world test:%d\r\n", 1);
#endif
    
    return pdPASS;
}

static BaseType_t usart_hardware_init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
        return pdFAIL;
    
    //start usart1 interrupt.
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    HAL_NVIC_EnableIRQ(USART1_IRQn);			
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 1);	
    
    return pdPASS;    
}

extern BaseType_t logger_send_data(uint8_t data);
void USART1_IRQHandler(void)
{
    uint8_t rx_data;
    
    if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
    {
        if(HAL_UART_Receive(&huart1, &rx_data, 1, 100) == HAL_OK)
        {
            logger_send_data(rx_data);
        }
    }
}

//define fputc used for printf remap.
FILE __stdout;   
void _sys_exit(int x) 
{ 
    x = x; 
} 
int fputc(int ch, FILE *f)
{ 	
#if LOGGER_DEFAULT_INTERFACE == LOGGER_INTERFACE_UART
    usart1_translate((char *)&ch, 1); 
#else
    ITM_SendChar(ch);
#endif
    return ch;
}
