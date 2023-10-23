//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_spi.c
//
//  Purpose:
//      spi driver for flash.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_spi.h"
#include "drv_soft_spi.h"

#define SPI_RW_TIMEOUT  100

static uint16_t spi_id = 0;

//spi interface
SPI_HandleTypeDef hspi5;

//wq interface
static void wq_wait_busy(void);

#if SPI_RUN_MODE == SPI_USE_HARDWARE_POLL
BaseType_t spi_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_SPI5_CLK_ENABLE();
    
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    
    /*Configure GPIO pin : PF6 */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    hspi5.Instance = SPI5;
    hspi5.Init.Mode = SPI_MODE_MASTER;
    hspi5.Init.Direction = SPI_DIRECTION_2LINES;
    hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi5.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi5.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi5.Init.NSS = SPI_NSS_SOFT;
    hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi5.Init.CRCPolynomial = 10;

    if (HAL_SPI_Init(&hspi5) != HAL_OK)
        return pdFAIL;

    return pdPASS;  
}

uint8_t spi_rw_byte(uint8_t data, HAL_StatusTypeDef *err)
{
    uint8_t rx_data = 0xff;
    HAL_StatusTypeDef status;
    
    status = HAL_SPI_TransmitReceive(&hspi5, &data, &rx_data, 1, SPI_RW_TIMEOUT); 
    if(err != NULL)
    {
        *err = status;
    }
    
    return rx_data;
}
#elif SPI_RUN_MODE == SPI_USE_HARDWARE_DMA

static DMA_HandleTypeDef hdma_spi5_rx;
static DMA_HandleTypeDef hdma_spi5_tx;

BaseType_t spi_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_SPI5_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    
    /*Configure GPIO pin : PF6 */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    hspi5.Instance = SPI5;
    hspi5.Init.Mode = SPI_MODE_MASTER;
    hspi5.Init.Direction = SPI_DIRECTION_2LINES;
    hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi5.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi5.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi5.Init.NSS = SPI_NSS_SOFT;
    hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi5.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi5) != HAL_OK)
        return pdFAIL;

    hdma_spi5_rx.Instance = DMA2_Stream3;
    hdma_spi5_rx.Init.Channel = DMA_CHANNEL_2;
    hdma_spi5_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi5_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi5_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi5_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi5_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi5_rx.Init.Mode = DMA_NORMAL;
    hdma_spi5_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_spi5_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_spi5_rx) != HAL_OK)
        return pdFAIL;
    
    __HAL_LINKDMA(&hspi5,hdmarx,hdma_spi5_rx);

    /* SPI5_TX Init */
    hdma_spi5_tx.Instance = DMA2_Stream4;
    hdma_spi5_tx.Init.Channel = DMA_CHANNEL_2;
    hdma_spi5_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi5_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi5_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi5_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi5_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi5_tx.Init.Mode = DMA_NORMAL;
    hdma_spi5_tx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_spi5_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_spi5_tx) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_LINKDMA(&hspi5, hdmatx, hdma_spi5_tx);
    
    __HAL_SPI_ENABLE(&hspi5);
    
    SET_BIT(hspi5.Instance->CR2, SPI_CR2_TXDMAEN); 
    SET_BIT(hspi5.Instance->CR2, SPI_CR2_RXDMAEN);
    return pdPASS;  
}

uint8_t spi_rw_byte(uint8_t data, HAL_StatusTypeDef *err)
{
    uint8_t rx_data = 0xff;
    HAL_StatusTypeDef status;
    
    status = HAL_SPI_TransmitReceive(&hspi5, &data, &rx_data, 1, SPI_RW_TIMEOUT); 
    if(err != NULL)
    {
        *err = status;
    }
    return rx_data;
}

uint8_t spi_write_dma(uint8_t *data, uint16_t size)
{
    //when tx, stop dma rx
    __HAL_DMA_CLEAR_FLAG(&hdma_spi5_tx, DMA_FLAG_TCIF0_4);
    __HAL_DMA_DISABLE(&hdma_spi5_rx);    
    __HAL_DMA_DISABLE(&hdma_spi5_tx);
    
    //dma start tx translate
    return HAL_DMA_Start(&hdma_spi5_tx, (uint32_t)data, (uint32_t)&hspi5.Instance->DR, size);
}

uint8_t spi_write_check_ok(void)
{
    if(__HAL_DMA_GET_FLAG(&hdma_spi5_tx, DMA_FLAG_TCIF0_4) != RESET)
    {
        __HAL_DMA_CLEAR_FLAG(&hdma_spi5_tx, DMA_FLAG_TCIF0_4);
        return HAL_OK;
    }
    return HAL_ERROR;
}

uint8_t spi_read_dma(uint8_t *data, uint16_t size)
{
    //close dma tx and rx
    __HAL_DMA_CLEAR_FLAG(&hdma_spi5_tx, DMA_FLAG_TCIF0_4);
    __HAL_DMA_CLEAR_FLAG(&hdma_spi5_rx, DMA_FLAG_TCIF3_7);
    __HAL_DMA_DISABLE(&hdma_spi5_rx);
    __HAL_DMA_DISABLE(&hdma_spi5_tx);
    
    //dma start rx read
    HAL_DMA_Start(&hdma_spi5_rx, (uint32_t)&hspi5.Instance->DR, (uint32_t)data, size);
    
    //dma start tx translate
    return HAL_DMA_Start(&hdma_spi5_tx, (uint32_t)&hspi5.Instance->DR, (uint32_t)data, size);
}

uint8_t spi_read_check_ok(void)
{
    if(__HAL_DMA_GET_FLAG(&hdma_spi5_rx, DMA_FLAG_TCIF3_7) != RESET)
    {
        __HAL_DMA_CLEAR_FLAG(&hdma_spi5_tx, DMA_FLAG_TCIF0_4);
        __HAL_DMA_CLEAR_FLAG(&hdma_spi5_rx, DMA_FLAG_TCIF3_7);
        return HAL_OK;
    }
    
    return HAL_ERROR;
}

#else
BaseType_t spi_driver_init(void)
{
    SOFT_SPI_INFO spi_info = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
       
    __HAL_RCC_GPIOF_CLK_ENABLE();  
    
    spi_info.mode = SPI_MODE_CPOL_H_CPHA_2;
    spi_info.sck_pin = GPIO_PIN_7;
    spi_info.sck_port = GPIOF;
    spi_info.miso_pin = GPIO_PIN_8;
    spi_info.miso_port = GPIOF;
    spi_info.mosi_pin = GPIO_PIN_9;
    spi_info.mosi_port = GPIOF;
    
    if(spi_soft_init(SOFT_SPI5, &spi_info) != SPI_OK)
    {
        return pdFAIL;
    }
    
    //cs pin
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);
    
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    
    return pdPASS;
}

uint8_t spi_rw_byte(uint8_t data, HAL_StatusTypeDef *err)
{
    uint8_t rdata = 0xff;
    uint8_t spi_status;
    
    spi_status = spi_soft_rw_byte(SOFT_SPI5, &data, &rdata, 1);
    if(err != NULL)
    {
        if(spi_status == SPI_OK)
        {
            *err = HAL_OK;
        }
        else
        {
            *err = HAL_ERROR;
        }
    }
    return rdata;
}
#endif
