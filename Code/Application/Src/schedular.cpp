
#include "schedular.hpp"
#include "logger.hpp"
#include "driver.hpp"

bool schedular::init(void)
{
    BaseType_t xReturned;
    
    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    run,       /* Function that implements the task. */
                    "schedular",          /* Text name for the task. */
                    SCHEDULAR_TASK_STACK,                    /* Stack size in words, not bytes. */
                    ( void * ) NULL,    /* Parameter passed into the task. */
                    SCHEDULAR_TASK_PROITY,/* Priority at which the task is created. */
                    &task_handle_ );      /* Used to pass out the created task's handle. */
    
   //wwdg_init();    
                    
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

void schedular::run(void* parameter)
{    
    //tell driver os is start.
    set_os_on();
    
    //fault_test_by_unalign();
    
    while(1)
    {
        //PRINT_LOG(LOG_INFO, xTaskGetTickCount(), "LED Task Run!");
        LED0_ON;
        vTaskDelay(100);
        LED0_OFF;
        vTaskDelay(100);
        
        schedular::get_instance()->wwdg_reload();   
    }    
}

void schedular::wwdg_reload(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}

//wwdg clock used LSI = 32khz
//times = 4065* 1/(32khz/32) = 4s
void schedular::wwdg_init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
    hiwdg.Init.Reload = 4095;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        Error_Handler();
    }
}
