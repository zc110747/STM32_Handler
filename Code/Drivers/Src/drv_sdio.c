//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_sdio.c
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

#if SDIO_RUN_MODE == SDIO_MODE_PULL
static SD_HandleTypeDef hsdcard1;

BaseType_t sdcard_driver_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    //clock enable
    __HAL_RCC_SDIO_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
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

    sdcard_driver_test();

    return pdPASS;
}

HAL_StatusTypeDef sdcard_read_disk(uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint16_t tick = 0;
    
    taskENTER_CRITICAL();
    status = HAL_SD_ReadBlocks(&hsdcard1, (uint8_t*)buf, startBlocks, NumberOfBlocks, SDMMC_READ_WRITE_TIMEOUT);
    taskEXIT_CRITICAL();
    
    //wait card ok.
    while((HAL_SD_GetCardState(&hsdcard1) != HAL_SD_CARD_TRANSFER)
    && (tick < SDMMC_READ_WRITE_TIMEOUT))
    {
        delay_ms(1);
        tick++;
    }
    if(tick >= SDMMC_READ_WRITE_TIMEOUT)
    {
        return HAL_TIMEOUT;
    }
    return status;
}

HAL_StatusTypeDef sdcard_write_disk(const uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint16_t tick = 0;

    taskENTER_CRITICAL();
    status = HAL_SD_WriteBlocks(&hsdcard1, (uint8_t*)buf, startBlocks, NumberOfBlocks, SDMMC_READ_WRITE_TIMEOUT);
    taskEXIT_CRITICAL();
    
    //wait card ok.
    while((HAL_SD_GetCardState(&hsdcard1) != HAL_SD_CARD_TRANSFER)
    && (tick < SDMMC_READ_WRITE_TIMEOUT))
    {
        delay_ms(1);
        tick++;
    }
    if(tick >= SDMMC_READ_WRITE_TIMEOUT)
    {
        return HAL_TIMEOUT;
    }

    return status;
}
#endif

BaseType_t sdcard_driver_test(void)
{
#if SDMMC_TEST == 1   
    uint8_t buffer[512] = {0};
    char *ptr = (char *)"sdmmc test success\r\n";
    uint16_t index = 0;
    

    strcpy((char *)buffer, ptr);
    
    for(index=0; index<20000; index++)
    {
        if(write_disk(buffer, index, 1) == HAL_OK)
        {       
            memset(buffer, 0, 512);
            if(read_disk(buffer, index, 1) == HAL_OK)
            {
                printf("block%d:%s", index, buffer);
            }
            else
                return pdFAIL;
        }
        else
        {   
            printf("block write failed:%d", index);
            continue;
        }
    }
#endif
    
    return pdPASS;
}
