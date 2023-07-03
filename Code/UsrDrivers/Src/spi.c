//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      spi.cpp
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
#include "spi.h"

#define SPI_RW_TIMEOUT  100

static uint16_t spi_id = 0;

//spi interface
SPI_HandleTypeDef spi_handler_;
static BaseType_t spi_hardware_init(void);
static BaseType_t spi_test(void);

//wq interface
static void wq_wait_busy(void);

//spi hardware
BaseType_t spi_init()
{
    BaseType_t result;
    
    result = spi_hardware_init();
    if(result == pdPASS)
    {
        spi_test();
    }
    return result;  
}

static BaseType_t spi_test(void)
{
    return pdPASS;
}

static BaseType_t spi_hardware_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  __HAL_RCC_GPIOF_CLK_ENABLE();
  
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);
  
  /*Configure GPIO pin : PF6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  spi_handler_.Instance = SPI5;
  spi_handler_.Init.Mode = SPI_MODE_MASTER;
  spi_handler_.Init.Direction = SPI_DIRECTION_2LINES;
  spi_handler_.Init.DataSize = SPI_DATASIZE_8BIT;
  spi_handler_.Init.CLKPolarity = SPI_POLARITY_HIGH;
  spi_handler_.Init.CLKPhase = SPI_PHASE_2EDGE;
  spi_handler_.Init.NSS = SPI_NSS_SOFT;
  spi_handler_.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  spi_handler_.Init.FirstBit = SPI_FIRSTBIT_MSB;
  spi_handler_.Init.TIMode = SPI_TIMODE_DISABLE;
  spi_handler_.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  spi_handler_.Init.CRCPolynomial = 10;
  
  if (HAL_SPI_Init(&spi_handler_) != HAL_OK)
    return pdFAIL;
  
  return pdPASS;
}


uint8_t spi_rw_byte(uint8_t data)
{
    uint8_t rx_data;
    
    HAL_SPI_TransmitReceive(&spi_handler_,&data, &rx_data, 1, SPI_RW_TIMEOUT); 
    
    return rx_data;
}

//wq interface
void wq_init(void)
{
    spi_id = wq_read_id();
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
    
    while((wq_read_sr(1)&0x01)==0x01);   
}


