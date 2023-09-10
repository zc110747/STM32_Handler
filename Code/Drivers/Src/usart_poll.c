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

#if USART_RUN_MODE == USART_MODE_POLL

//global parameter
static UART_HandleTypeDef huart1;
static uint8_t is_usart_driver_init = 0;

//global function
static void usart_run_test(void);

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

static void usart_run_test(void)
{
  uint16_t rx_data;

  while(1)
  {
    if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
    {
      //等待数据接收
      rx_data = UART_ReceiveData(&huart1);

      while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) == RESET)
      {}

      //将接收数据发送
      UART_SendData(&huart1, rx_data);
    }
  }
}

BaseType_t usart_driver_init(void)
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

  is_usart_driver_init = 1;
  
#if RUN_TEST_MODE == USART_TEST
  usart_translate("usart test for polling!\r\n", strlen("usart test for polling!\r\n"));
  usart_run_test();
#endif
  return pdPASS; 
}

void usart_translate(char *ptr, uint16_t size)
{
    uint16_t index;
    for(index=0; index<size; index++)
    {
      while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) == RESET)
      {}
      
      //数据发送
      UART_SendData(&huart1, ptr[index]);
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
  if(is_usart_driver_init == 1)
  {
    usart_translate((char *)&ch, 1); 
  }
  return ch;
}
#endif
