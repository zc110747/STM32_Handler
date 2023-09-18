//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      usart_dma.cpp
//      USART_TX -------------- PA9
//      USART_RX -------------- PA10
//          
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

#if USART_RUN_MODE == USART_MODE_DMA

//global define
#define DMA_BUFFER_SIZE  256

typedef struct
{
  uint8_t rx_size;
  uint8_t is_rx_dma;
}RX_INFO;

//global parameter
static UART_HandleTypeDef huart1;
static DMA_HandleTypeDef hdma_usart1_rx;
static DMA_HandleTypeDef hdma_usart1_tx;
static uint8_t is_usart_driver_init = 0;

static RX_INFO gRxInfo = {0};
static char dma_rx_buffer[DMA_BUFFER_SIZE];
static char dma_tx_buffer[DMA_BUFFER_SIZE];

//global function
static void usart_run_test(void);

BaseType_t usart_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    
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
    
    //enable uart idle interrupt
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    HAL_NVIC_EnableIRQ(USART1_IRQn);			
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);	
    
    ATOMIC_SET_BIT(huart1.Instance->CR3, USART_CR3_DMAT);
    ATOMIC_SET_BIT(huart1.Instance->CR3, USART_CR3_DMAR);
           
    //update dma communication
    hdma_usart1_rx.Instance = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_NORMAL;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_usart1_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_usart1_rx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_usart1_rx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      return pdFAIL;
    }
    __HAL_LINKDMA(&huart1, hdmarx, hdma_usart1_rx);
    
    hdma_usart1_tx.Instance = DMA2_Stream7;
    hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_usart1_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_usart1_tx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_usart1_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      return pdFAIL;
    }
    __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);
    
    //enable uart idle interrupt
    __HAL_DMA_ENABLE_IT(&hdma_usart1_tx, DMA_IT_TC);
    HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);			
    HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 0, 1);	
    
    //enable dma rx
    usart_receive(dma_rx_buffer, DMA_BUFFER_SIZE);
    
    is_usart_driver_init = 1;
    
#if RUN_TEST_MODE == USART_TEST
  usart_translate("usart test for polling!\r\n", strlen("usart test for polling!\r\n"));
  
  usart_run_test();
#endif
    
    return pdPASS; 
}

void usart_translate(char *ptr, uint16_t size)
{
    memcpy((char *)dma_tx_buffer, ptr, size);

    //clear the flag related to translate
    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_tx, DMA_FLAG_TCIF3_7);
    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_tx, DMA_FLAG_TCIF3_7);
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TC);

    //enable dma tc interrupt
    __HAL_DMA_ENABLE_IT(&hdma_usart1_tx, DMA_IT_TC);

    //start dma translate
    HAL_DMA_Start(&hdma_usart1_tx, (uint32_t)dma_tx_buffer, (uint32_t)&huart1.Instance->DR, size);
}

void usart_receive(char *ptr, uint16_t size)
{
    //clear dma rx flag
    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_tx, DMA_FLAG_TCIF2_6);
    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_tx, DMA_FLAG_TCIF2_6);

    //启动等待下次接收
    HAL_DMA_Start(&hdma_usart1_rx, (uint32_t)&huart1.Instance->DR, (uint32_t)ptr, DMA_BUFFER_SIZE);
}

static void usart_run_test(void)
{
  char data[DMA_BUFFER_SIZE];
  
  while(1)
  {
    if(gRxInfo.is_rx_dma == 1)
    {
        memcpy(data, dma_rx_buffer, gRxInfo.rx_size);
        memset(dma_rx_buffer, 0, DMA_BUFFER_SIZE);
        gRxInfo.is_rx_dma = 0;
        
        //启动下次数据接收
        usart_receive(dma_rx_buffer, DMA_BUFFER_SIZE);
        
        //发送接收到的数据
        usart_translate(data, gRxInfo.rx_size);
    }
  }
}

void DMA2_Stream7_IRQHandler(void)
{
    if(__HAL_DMA_GET_FLAG(&hdma_usart1_tx, DMA_FLAG_TCIF3_7) != RESET)
    {      
        //close the dma and all flags, also interrupt
        //need enable next
        HAL_DMA_Abort(&hdma_usart1_tx);   
    }
}

//usart中断接收处理
void USART1_IRQHandler(void)
{
    uint8_t data;
    
    if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET)
    {
      __HAL_UART_CLEAR_PEFLAG(&huart1);
      
      HAL_DMA_Abort(&hdma_usart1_rx);
      
      gRxInfo.is_rx_dma = 1;
      gRxInfo.rx_size = DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
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
    //usart_translate((char *)&ch, 1); 
#else
    ITM_SendChar(ch);
#endif
    return ch;
}
#endif
