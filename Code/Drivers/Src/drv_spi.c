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

uint8_t spi_multi_w_byte(uint8_t *data, uint8_t size)
{
    if(spi_soft_rw_byte(SOFT_SPI5, data, NULL, size) == SPI_OK)
    {
        return 0;
    }
    return 1;
}

#endif

//wq interface
BaseType_t spi_wq_driver_init(void)
{
    BaseType_t result;
    
    //spi
    result = spi_driver_init();
    
    if(result == pdPASS)
    {
        spi_id = wq_read_id();
        PRINT_LOG(LOG_INFO, "spi wq init success, id:0x%x!", spi_id);
    }
    else
    {
        PRINT_LOG(LOG_ERROR, "spi wq init failed!", spi_id); 
    }
    return result;
}

uint16_t wq_read_id(void)
{
    uint16_t id = 0;
    
    SPI_CS_ON;
    spi_rw_byte(0x90);
    spi_rw_byte(0x00);
    spi_rw_byte(0x00);
    spi_rw_byte(0x00);
    id = spi_rw_byte(0xff)<<8;
    id |= spi_rw_byte(0xff);
    SPI_CS_OFF; 
    
    return id;  
}

void wq_write_enable(void)
{
    SPI_CS_ON;
    spi_rw_byte(W25X_WriteEnable);
    SPI_CS_OFF; 
}

void wq_write_disable(void)
{
    SPI_CS_ON;
    spi_rw_byte(W25X_WriteDisable);
    SPI_CS_OFF;    
}

void wq_read(uint8_t *pbuffer, uint32_t addr, uint16_t num)
{
    uint16_t i;
    SPI_CS_ON;   
    spi_rw_byte(W25X_ReadData);
    if(spi_id == CHIP_ID_Q256)
    {
        spi_rw_byte((uint8_t)(addr>>24));
    }
    spi_rw_byte((uint8_t)(addr>>16));
    spi_rw_byte((uint8_t)(addr>>8));
    spi_rw_byte((uint8_t)(addr));
    for(i = 0; i<num; i++)
    {
       pbuffer[i] = spi_rw_byte(0xff);
    }
    SPI_CS_OFF;    
}

uint16_t wq_write_page(uint8_t *pbuffer, uint32_t addr, uint16_t num)
{
    uint16_t i;
    
    wq_write_enable();
    
    SPI_CS_ON;
    spi_rw_byte(W25X_PageProgram); 
    if(spi_id == CHIP_ID_Q256)          
    {
        spi_rw_byte((uint8_t)((addr)>>24)); 
    }
    spi_rw_byte((uint8_t)((addr)>>16)); 
    spi_rw_byte((uint8_t)((addr)>>8));   
    spi_rw_byte((uint8_t)addr);   
    for(i=0; i<num; i++)
    {
        spi_rw_byte(pbuffer[i]);
    }
    SPI_CS_OFF;
    
    wq_wait_busy();
    
    return num;
}

void wq_erase_sector(uint32_t sector)
{  
 	sector *= 4096;
    
    wq_write_enable();            
    wq_wait_busy();  
    
  	SPI_CS_ON;                          
    spi_rw_byte(W25X_SectorErase);   
    
    if(spi_id == CHIP_ID_Q256)                
    {
        spi_rw_byte((uint8_t)((sector)>>24)); 
    }
    spi_rw_byte((uint8_t)((sector)>>16));  
    spi_rw_byte((uint8_t)((sector)>>8));   
    spi_rw_byte((uint8_t)sector);  
    SPI_CS_OFF;      
    
    wq_wait_busy();   				   
}  

uint8_t wq_read_sr(uint8_t reg)
{
    uint8_t byte = 0;
    uint8_t command = 0;
    
    switch(reg)
    {
    case 1:
        command=W25X_ReadStatusReg1;    
        break;
    case 2:
        command=W25X_ReadStatusReg2;    
        break;
    case 3:
        command=W25X_ReadStatusReg3;
        break;
    default:
        command=W25X_ReadStatusReg1;    
        break;
    }    
    SPI_CS_ON;                           
    spi_rw_byte(command);            
    byte = spi_rw_byte(0Xff);       
    SPI_CS_OFF;                       
    
    return byte; 
}

static void wq_wait_busy(void)
{
    BaseType_t type = pdPASS;
    
    do
    {    
    }while((wq_read_sr(1) & 0x01) == 0x01);   
}


