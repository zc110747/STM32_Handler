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

#if SPI_RUN_MODE == SPI_USE_HARDWARE
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

uint8_t spi_rw_byte(uint8_t data)
{
    uint8_t rx_data = 0xff;
    
    HAL_SPI_TransmitReceive(&hspi5, &data, &rx_data, 1, SPI_RW_TIMEOUT); 
    
    return rx_data;
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

uint8_t spi_rw_byte(uint8_t data)
{
    uint8_t rdata = 0xff;
    
    spi_soft_rw_byte(SOFT_SPI5, &data, &rdata, 1);
    
    return rdata;
}
#endif
