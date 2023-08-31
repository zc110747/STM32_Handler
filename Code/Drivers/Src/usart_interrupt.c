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
#include "circular_buffer.h"

#if USART_RUN_MODE == USART_MODE_INTERRUPT

//global define
#define CIRCULAR_BUFFER_SIZE  256

//global parameter
static UART_HandleTypeDef huart1;
static uint8_t is_usart_init = 0;

static volatile CircularBuffer rx_circular;
static uint8_t  rx_cache[CIRCULAR_BUFFER_SIZE];
static volatile CircularBuffer tx_circular;
static uint8_t  tx_cache[CIRCULAR_BUFFER_SIZE];

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

BaseType_t usart_init(void)
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
      
  //开启USART接收非空中断和发送完成中断
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
  HAL_NVIC_EnableIRQ(USART1_IRQn);			
  HAL_NVIC_SetPriority(USART1_IRQn, 1, 1);	
 
  //initialize the buffer
  CircularBufferInit(&rx_circular, CIRCULAR_BUFFER_SIZE, rx_cache);
  CircularBufferInit(&tx_circular, CIRCULAR_BUFFER_SIZE, tx_cache);
  
  is_usart_init = 1;
  
#if RUN_TEST_MODE == USART_TEST
  usart_translate("usart test for polling!\r\n", strlen("usart test for polling!\r\n"));
  usart_run_test();
#endif

  return pdPASS; 
}

static void usart_run_test(void)
{
  char data;

  while(1)
  {
    //对于接收和发送因为都是由缓存处理，所以在主循环操作时，
    //即使其它地方导致延时2-3ms，仍然能够正确处理，从而保证数据不会丢失
    if(CircularBufferHasData(&rx_circular))
    {
      CircularBufferGet(&rx_circular, data);
      
      usart_translate(&data, 1);
    }
  }
}

//usart中断接收处理
void USART1_IRQHandler(void)
{
  uint16_t data;

  if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
  {
    data = UART_ReceiveData(&huart1);
    CircularBufferPut(&rx_circular, data);
  }

  //有关闭TXE的动作，当RXNE触发时，有可能误触发TXE
  //所以需要检测TXE是否开启
  if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) != RESET
  && __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TXE))
  {
    if(CircularBufferIsEmpty(&tx_circular))
    {
      __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
    }
    else
    {
      CircularBufferGet(&tx_circular, data);
      UART_SendData(&huart1, data);
      if(CircularBufferIsEmpty(&tx_circular))
      {
        __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
      }
    }
  }
}

void usart_translate(char *ptr, uint16_t size)
{
    uint16_t index;
    
    for(index=0; index<size; index++)
    {
    
      //如果检测到fifo满，则使用堵塞的方式从fifo取数据发送
      //正常工作发送设计上应该保证fifo不应该满，如果满说明发送效率太低或者FIFO定义不合理
      //此时建议提高波特率、降低发送频率或者加大FIFO三方面进行调试
      if(CircularBufferIsFull(&tx_circular))
      {
        uint8_t data;

        __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
        CircularBufferGet(&tx_circular, data);
        
        while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) == RESET)
        {}
        UART_SendData(&huart1, data);
      }

      __disable_irq();
      CircularBufferPut(&tx_circular, ptr[index]);
      __enable_irq();

      __HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
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
  if(is_usart_init == 1)
  {
    usart_translate((char *)&ch, 1); 
  }
  return ch;
}
#endif
