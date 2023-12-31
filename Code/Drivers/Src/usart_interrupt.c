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
#include "drv_usart.h"
#include "logger_process.h"

#if LOGGER_ON == 1 && USART_RUN_MODE != USART_MODE_INTERRUPT
#error "LOGGER interface use uart interrupt mode, not support poll or dma, fix by set LOGGER_ON=0 or change USART_RUN_MODE".
#endif

#if USART_RUN_MODE == USART_MODE_INTERRUPT 

//global define
#define CIRCULAR_BUFFER_SIZE  256

//global parameter
static UART_HandleTypeDef huart1;

uint16_t UART_ReceiveData(UART_HandleTypeDef* huart)
{
    uint16_t rx_data;

    rx_data = huart->Instance->DR&0x1FF;

    return rx_data;
}

void UART_SendData(UART_HandleTypeDef* huart, uint16_t Data)
{
    huart->Instance->DR = Data&0x1FF;
}

BaseType_t usart_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
      
    //开启USART接收非空中断和发送完成中断
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);

    HAL_NVIC_SetPriority(USART1_IRQn, 1, 1);	
    HAL_NVIC_EnableIRQ(USART1_IRQn);			

    return pdPASS; 
}

GlobalType_t uart_logger_write(uint8_t *ptr, uint8_t size)
{
    GlobalType_t res;

    __disable_irq();
    res = logger_put_tx_buffer(LOG_DEVICE_USART, ptr, size);  
    __enable_irq();

    if(res == GLOBAL_OK)
    {
      __HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
    }
    return res;
}

void USART1_IRQHandler(void)
{
    uint8_t data;
    
    if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
    {
        data = UART_ReceiveData(&huart1);
        logger_put_rx_buffer(LOG_DEVICE_USART, &data, 1);
    }
    
    if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) != RESET
    && __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TXE))
    {
        if(logger_get_tx_byte(LOG_DEVICE_USART, &data) == GLOBAL_ERROR)
        {
            __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
        }
        else
        {
            UART_SendData(&huart1, data);
        }
    }
}
#endif

//define fputc used for printf remap.
FILE __stdout;   
void _sys_exit(int x) 
{ 
    x = x; 
} 
int fputc(int ch, FILE *f)
{ 	
  uart_logger_write((uint8_t *)&ch, 1); 
  return ch;
}
