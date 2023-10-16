//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      dma.c
//
//  Purpose:
//      dma application.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_dma.h"

#define DMA_DST_SIZE        256
#define DMA_SRC_ADDRESS     0x8000000

static uint8_t dma_dst[DMA_DST_SIZE];
static DMA_HandleTypeDef hdma_memtomem;

static BaseType_t dma_memory_run(void);

#define DMA_MEM_STREAM  DMA2_Stream1
#define DMA_MEM_FLAG    DMA_FLAG_TCIF1_5

BaseType_t dma_driver_init(void)
{
    BaseType_t xReturn = pdFAIL;

    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* Configure DMA request hdma_memtomem on DMA2_Stream0 */
    hdma_memtomem.Instance = DMA_MEM_STREAM;
    hdma_memtomem.Init.Channel = DMA_CHANNEL_0;
    hdma_memtomem.Init.Direction = DMA_MEMORY_TO_MEMORY;
    hdma_memtomem.Init.PeriphInc = DMA_PINC_ENABLE;
    hdma_memtomem.Init.MemInc = DMA_MINC_ENABLE;
    hdma_memtomem.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_memtomem.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_memtomem.Init.Mode = DMA_NORMAL;
    hdma_memtomem.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_memtomem.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_memtomem.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_memtomem.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_memtomem.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_memtomem) != HAL_OK)
    return pdFAIL;

    xReturn = dma_memory_run();
    return xReturn;
}

BaseType_t dma_translate(uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
    if(HAL_DMA_Start(&hdma_memtomem, SrcAddress, DstAddress, DataLength) != HAL_OK)
        return pdFAIL;
    
    return pdPASS;
}

BaseType_t dma_check_finish(uint32_t timeout_ms)
{
    BaseType_t xReturn = pdFAIL;
    uint32_t index = 0;
    
    if(timeout_ms == 0)
    {
        if(__HAL_DMA_GET_FLAG(&hdma_memtomem, DMA_MEM_FLAG) == SET)
        {
            xReturn = pdPASS;
        }
    }
    else
    {
       while(__HAL_DMA_GET_FLAG(&hdma_memtomem, DMA_MEM_FLAG) == RESET)
       {
           index++;
           if(index >= timeout_ms)
           {
               break;
           }
           delay_ms(timeout_ms);
       }
       
       if(index<timeout_ms)
           xReturn = pdPASS;
    }
    
    return xReturn;
}

static BaseType_t dma_memory_run(void)
{
    BaseType_t xReturn = pdFAIL;
    
    dma_translate(DMA_SRC_ADDRESS, (uint32_t)dma_dst, DMA_DST_SIZE);
  
    if(dma_check_finish(100) == pdPASS)
    {
        __HAL_DMA_CLEAR_FLAG(&hdma_memtomem, DMA_MEM_FLAG);
        
        if(memcmp((char *)DMA_SRC_ADDRESS, dma_dst, DMA_DST_SIZE) == 0)
        {
            PRINT_LOG(LOG_INFO, "dma memory to memory test success!");
            xReturn = pdPASS;
        }
        else
        {
            PRINT_LOG(LOG_INFO, "dma memory to memory test data not equal!");
        }
    }
    else
    {
        PRINT_LOG(LOG_INFO, "dma memory to memory test not finish!");
    }
    
    return xReturn;  
}