//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      drv_can.c
//
//  Purpose:
//      can driver for init, read, write
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_can.h"
#include "circular_buffer.h"

static CAN_HandleTypeDef hcan1;
static CAN_RxHeaderTypeDef can_rx_header;
static CAN_TxHeaderTypeDef can_tx_header;
static uint8_t can_rx_msg[8];
static uint8_t can_tx_msg[8];

#define CAN_BUFFER_SIZE  256

static uint8_t can_rx_buffer[CAN_BUFFER_SIZE];
static volatile CircularBuffer CanRxBufferInfo;

BaseType_t drv_can_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 16;
    hcan1.Init.Mode = CAN_MODE_LOOPBACK;
    hcan1.Init.SyncJumpWidth = CAN_SJW_2TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_3TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_3TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = ENABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = DISABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan1) != HAL_OK)
        return pdFAIL; 
    
    CAN_FilterTypeDef CAN_FilterTypeDefSture;

    CAN_FilterTypeDefSture.FilterBank  = 0;                          
    CAN_FilterTypeDefSture.FilterScale = CAN_FILTERSCALE_16BIT;         
    CAN_FilterTypeDefSture.FilterMode  = CAN_FILTERMODE_IDMASK;        
    //ID is 0x00£¬mask is 0x00£¬not fiter
    CAN_FilterTypeDefSture.FilterIdLow      = 0x00;                     //FR1
    CAN_FilterTypeDefSture.FilterMaskIdLow  = 0x00;
    CAN_FilterTypeDefSture.FilterIdHigh     = 0x00;                     //FR2
    CAN_FilterTypeDefSture.FilterMaskIdHigh = 0x00;   
    CAN_FilterTypeDefSture.FilterFIFOAssignment = CAN_FILTER_FIFO0;     //Fiter 0 assignment to FIFO0
    CAN_FilterTypeDefSture.FilterActivation = ENABLE;                   //start Fiter 0
    CAN_FilterTypeDefSture.SlaveStartFilterBank = 14;

    if(HAL_CAN_ConfigFilter(&hcan1, &CAN_FilterTypeDefSture) != HAL_OK)
    {
        return pdFAIL;
    }
    
    if(HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
        return pdFAIL;
 
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn,1,2); 
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    
    CircularBufferInit(&CanRxBufferInfo, CAN_BUFFER_SIZE, can_rx_buffer);
    HAL_CAN_Start(&hcan1);
    return pdPASS;
}

BaseType_t can_driver_write(uint8_t *msg, uint8_t size)
{
    uint8_t i, err_times = 0;
    uint32_t mailbox;
    
    can_tx_header.StdId = 0x12;  //stdard id for can
    can_tx_header.ExtId = 0x00;  //extend id for can
    can_tx_header.IDE = CAN_ID_STD;
    can_tx_header.RTR = CAN_RTR_DATA;
    can_tx_header.TransmitGlobalTime = DISABLE;
    can_tx_header.DLC = size;
    
    //wait until with empty fifo.
    err_times = 0;
    while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 3)
    {
        delay_ms(1);
        err_times++;
        if(err_times > 100)
        {
            return pdFAIL;
        }
    }
    
    HAL_CAN_AddTxMessage(&hcan1, &can_tx_header, msg, &mailbox);
    return pdPASS;
}

void CAN1_RX0_IRQHandler(void)
{
    uint32_t interrupts = READ_REG(hcan1.Instance->IER);
    uint8_t index;
    
    if(interrupts&CAN_IT_RX_FIFO0_MSG_PENDING)
    {
        //fifo rx data can read
        while(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &can_rx_header, can_rx_msg) == HAL_OK)
        {
            for(index=0; index<can_rx_header.DLC; index++)
            {
                CircularBufferPut(&CanRxBufferInfo, can_rx_msg[index]);
            }
        }
    }
}

