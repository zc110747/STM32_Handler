//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_sdram.c
//
//  Purpose:
//      sdram driver by fsmc.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_sdram.h"

SDRAM_HandleTypeDef hsdram1;
 
static void sdram_memory_test(void);
static BaseType_t sdram_hardware_init(void);
static BaseType_t sdram_initialize_sequence(void);
static BaseType_t sdram_send_command(uint8_t bank, uint8_t cmd, uint8_t refresh, uint16_t regval);

BaseType_t sdram_driver_init(void)
{
    BaseType_t result;
    
    result =  sdram_hardware_init();
    if(result == pdPASS)
    {
        //initialize sdram command sequence
        result = sdram_initialize_sequence();

        if(result == pdPASS)
        {
            //define sdram refresh rate.
            HAL_SDRAM_ProgramRefreshRate(&hsdram1, 683);
            
            sdram_memory_test();
        }
        else
        {
            PRINT_LOG(LOG_INFO, "sdram_initialize_sequence failed!");
        }
    }
    else
    {
        PRINT_LOG(LOG_INFO, "sdram_hardware_init failed!");
    }

    PRINT_LOG(LOG_INFO, "sdram_hardware_init success!");
    return result;
}	

static BaseType_t sdram_hardware_init(void)
{
    FMC_SDRAM_TimingTypeDef SdramTiming = {0};
    GPIO_InitTypeDef GPIO_InitStruct ={0};
  
    __HAL_RCC_FMC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();   
    __HAL_RCC_GPIOG_CLK_ENABLE();   
    __HAL_RCC_GPIOE_CLK_ENABLE();    
    __HAL_RCC_GPIOD_CLK_ENABLE();  
    
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_12
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
    hsdram1.Instance = FMC_SDRAM_DEVICE;
    /* hsdram1.Init */
    hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
    hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
    hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;
    hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
    hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
    hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
    hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
    hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_1;

    /* SdramTiming */
    SdramTiming.LoadToActiveDelay = 2;
    SdramTiming.ExitSelfRefreshDelay = 8;
    SdramTiming.SelfRefreshTime = 6;
    SdramTiming.RowCycleDelay = 6;
    SdramTiming.WriteRecoveryTime = 2;
    SdramTiming.RPDelay = 2;
    SdramTiming.RCDDelay = 2;

    if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
        return pdFAIL;
    
    return pdPASS;
}

static BaseType_t sdram_initialize_sequence(void)
{
    uint32_t temp;
    BaseType_t result;

    result = sdram_send_command(0, FMC_SDRAM_CMD_CLK_ENABLE, 1, 0);
    HAL_Delay(1);
    result &= sdram_send_command(0, FMC_SDRAM_CMD_PALL, 1, 0);
    result &= sdram_send_command(0, FMC_SDRAM_CMD_AUTOREFRESH_MODE, 1, 0);

    temp = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1
            | SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL
            | SDRAM_MODEREG_CAS_LATENCY_3           
            | SDRAM_MODEREG_OPERATING_MODE_STANDARD 
            | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;  

    result &= sdram_send_command(0, FMC_SDRAM_CMD_LOAD_MODE, 1, temp);
    
    return result;
}

static BaseType_t sdram_send_command(uint8_t bank, uint8_t cmd, uint8_t refresh, uint16_t regval)
{
    uint32_t target_bank=0;
    FMC_SDRAM_CommandTypeDef Command;

    if(bank == 0) 
    {
        target_bank = FMC_SDRAM_CMD_TARGET_BANK1;
    }		
    else if(bank==1) 
    {
        target_bank = FMC_SDRAM_CMD_TARGET_BANK2;
    }

    Command.CommandMode = cmd;             
    Command.CommandTarget = target_bank;     
    Command.AutoRefreshNumber = refresh;    
    Command.ModeRegisterDefinition = regval;  
    if(HAL_SDRAM_SendCommand(&hsdram1, &Command, 0x1000) != HAL_OK)
        return pdFAIL;
    
    return pdPASS;
}

//0xc0000000 is the start of memory address for sdram. 
//address can be use after the test
static uint32_t test_sdram[100] __attribute__((section(".ARM.__at_0xC0000000")));

static void sdram_memory_test(void)
{
    uint32_t i;
    
    memset(test_sdram, 0, 100);
    for(i=0; i<100; i++)
    {
        test_sdram[i] = i;
    }

    for(i=0; i<100; i++)
    {
        if(test_sdram[i] != i)
        {
            PRINT_LOG(LOG_INFO, "sdram test failed:%d, %d", test_sdram[i], i);
            break;
        }
    }
}


