//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      schdeular.c
//
//  Purpose:
//      task used run lower period task.
//
//  Author:
//      @zc
//
//  Assumptions:
//	
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "schedular.hpp"
#include "driver.h"
#include "lwip.h"
#include "i2c_monitor.hpp"

bool schedular::init(void)
{
    BaseType_t xReturned;
    
    xReturned = xTaskCreate(run, "schedular", 
                    SCHEDULAR_TASK_STACK, (void *)NULL,  
                    SCHEDULAR_TASK_PROITY, &task_handle_);
                    
   if(xReturned == pdPASS)
       return true;
   return false;
}

void fault_test_by_unalign(void) 
{
    volatile int * SCB_CCR = (volatile int *) 0xE000ED14; // SCB->CCR
    volatile int * p;
    volatile int value;

    *SCB_CCR |= (1 << 3); /* bit3: UNALIGN_TRP. */

    p = (int *) 0x00;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);

    p = (int *) 0x04;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);

    p = (int *) 0x03;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);
}


void ETH_Reset(void)
{
#if I2C_MONITOR_MODULE_STATE  == MODULE_ON
    i2c_monitor::get_instance()->pcf8574_write_io(OUTPUT_ETH_RESET, IO_OFF);
    vTaskDelay(200);
    i2c_monitor::get_instance()->pcf8574_write_io(OUTPUT_ETH_RESET, IO_ON);
    vTaskDelay(200);
#else
    pcf8574_i2c_write(0xFF);
    vTaskDelay(100);
    pcf8574_i2c_write(0x7F);
    vTaskDelay(100);
#endif
}

void schedular::run(void* parameter)
{    
    //tell driver os is start.
    set_os_on();
    
    //reset the phy hardware
    ETH_Reset();
    
    MX_LWIP_Init();
      
    //PRINT_NOW("application init\r\n");
    PRINT_LOG(LOG_INFO, "application init success!");
    PRINT_LOG(LOG_INFO, "dbgmcu_id:0x%x", 
        DBGMCU->IDCODE);
    PRINT_LOG(LOG_INFO, "uid:0x%x, 0x%x, 0x%x",
        HAL_GetUIDw0(), HAL_GetUIDw1(), HAL_GetUIDw2());
    
    //fault_test_by_unalign();
    
    while(1)
    {
        //PRINT_LOG(LOG_INFO, xTaskGetTickCount(), "LED Task Run!");
        LED0_ON;        
        vTaskDelay(200);       
        LED0_OFF;       
        vTaskDelay(200);               
    }    
}
