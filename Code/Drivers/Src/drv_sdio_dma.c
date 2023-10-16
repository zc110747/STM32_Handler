//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_sdio_dma.c
//
//  Purpose:
//      sdcard driver for init, read, write.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_sdio.h"

#if SDIO_RUN_MODE == SDIO_MODE_DMA

static SD_HandleTypeDef hsdcard1;
static DMA_HandleTypeDef hdma_sdio_rx;
static DMA_HandleTypeDef hdma_sdio_tx;

BaseType_t sdcard_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    //clock enable
    __HAL_RCC_SDIO_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    
    /**SDIO GPIO Configuration
    PC8     ------> SDIO_D0
    PC9     ------> SDIO_D1
    PC10     ------> SDIO_D2
    PC11     ------> SDIO_D3
    PC12     ------> SDIO_CK
    PD2     ------> SDIO_CMD
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    //module init
    hsdcard1.Instance = SDIO;
    hsdcard1.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    hsdcard1.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    hsdcard1.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    hsdcard1.Init.BusWide = SDIO_BUS_WIDE_1B;
    hsdcard1.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsdcard1.Init.ClockDiv = SDMMC_CLOCK_DIV;

    if (HAL_SD_Init(&hsdcard1) != HAL_OK)
        return pdFAIL;

    if (HAL_SD_ConfigWideBusOperation(&hsdcard1, SDIO_BUS_WIDE_4B) != HAL_OK)
        return pdFAIL;

    hdma_sdio_rx.Instance = DMA2_Stream6;
    hdma_sdio_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdio_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sdio_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdio_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdio_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdio_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdio_rx.Init.Mode = DMA_PFCTRL;
    hdma_sdio_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_sdio_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdio_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdio_rx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdio_rx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdio_rx) != HAL_OK)
    {
        return pdFAIL;
    }
    __HAL_LINKDMA(&hsdcard1, hdmarx, hdma_sdio_rx);

    /* SDIO_TX Init */
    hdma_sdio_tx.Instance = DMA2_Stream3;
    hdma_sdio_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdio_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_sdio_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdio_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdio_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdio_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdio_tx.Init.Mode = DMA_PFCTRL;
    hdma_sdio_tx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_sdio_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdio_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdio_tx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdio_tx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdio_tx) != HAL_OK)
    {
        return pdFAIL;
    }
    __HAL_LINKDMA(&hsdcard1, hdmatx, hdma_sdio_tx);

    sdcard_driver_test();

    return pdPASS;
}


HAL_StatusTypeDef sdcard_read_disk(uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint16_t tick = 0;
    
    status = HAL_SD_ReadBlocks_DMA(&hsdcard1, (uint8_t*)buf, startBlocks, NumberOfBlocks);
    if(status == HAL_OK)
    {
        //wait card ok.
        while((HAL_SD_GetCardState(&hsdcard1) != HAL_SD_CARD_TRANSFER)
        && (tick < SDMMC_READ_WRITE_TIMEOUT))
        {
            delay_ms(1);
            tick++;
        }
        
        if(tick >= SDMMC_READ_WRITE_TIMEOUT)
        {
            status = HAL_TIMEOUT;
        }
    }
   
    hsdcard1.State = HAL_SD_STATE_READY;
    HAL_DMA_Abort_IT(&hdma_sdio_rx);
    __HAL_UNLOCK(&hdma_sdio_rx);
    hdma_sdio_rx.State = HAL_DMA_STATE_READY;
    return status;
}

HAL_StatusTypeDef sdcard_write_disk(const uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint16_t tick = 0;

    status = HAL_SD_WriteBlocks_DMA(&hsdcard1, (uint8_t*)buf, startBlocks, NumberOfBlocks);
    if(status == HAL_OK)
    {
        //wait card ok.
        while((HAL_SD_GetCardState(&hsdcard1) != HAL_SD_CARD_TRANSFER)
        && (tick < SDMMC_READ_WRITE_TIMEOUT))
        {
            delay_ms(1);
            tick++;
        }
        if(tick >= SDMMC_READ_WRITE_TIMEOUT)
        {
            status = HAL_TIMEOUT;
        }
    }
    
    hsdcard1.State = HAL_SD_STATE_READY;
    HAL_DMA_Abort_IT(&hdma_sdio_tx);
    __HAL_UNLOCK(&hdma_sdio_tx);
    hdma_sdio_tx.State = HAL_DMA_STATE_READY;
    return status;
}
#endif
