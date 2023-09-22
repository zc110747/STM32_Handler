//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     i2c_monitor.cpp
//
//  Purpose:
//     i2c montion driver.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "driver.hpp"
#include "i2c_monitor.hpp"

TaskHandle_t i2c_monitor::task_handle_{nullptr};
QueueHandle_t i2c_monitor::queue_{nullptr};
io_ex_info i2c_monitor::pcf8574_read_data_{0x00};
io_ex_info i2c_monitor::pcf8574_write_data_{0xff};   
 
BaseType_t i2c_monitor::init()
{
    BaseType_t xReturn;
    
    xReturn = xTaskCreate(
                    run,      
                    "i2c_monitor",        
                    I2C_MONITOR_TASK_STACK,              
                    ( void * ) NULL,   
                    I2C_MONITOR_TASK_PROITY,
                    &task_handle_ );      
  
    queue_ = xQueueCreate(i2c_monitor_MAX_QUEUE, sizeof(i2c_event));
    if(queue_ == nullptr)
    {
        xReturn = pdFAIL;
    }
    
    return xReturn;
}

BaseType_t i2c_monitor::trigger(uint8_t event, uint8_t *pdata, uint8_t size)
{
    i2c_event xdata;
    
    xdata.id = event;
    if(size > 0)
    {
        xdata.data = pdata[0];
    }
    return xQueueSend(queue_, &xdata, (TickType_t)1);
}

BaseType_t i2c_monitor::trigger_isr(uint8_t event, uint8_t *pdata, uint8_t size)
{
    i2c_event xdata;
    BaseType_t xReturn;
    portBASE_TYPE xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = pdFALSE;
    
    xdata.id = event;
    if(size > 0)
    {
        xdata.data = pdata[0];
    }
    
    /*send message*/
	xReturn = xQueueSendFromISR(queue_, &xdata, &xHigherPriorityTaskWoken);

	if(xHigherPriorityTaskWoken){
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

	return xReturn;
}

void i2c_monitor::pcf8574_write_io(uint8_t pin, uint8_t status)
{
    uint8_t io_status = status == IO_ON?0:1;
    
    portENTER_CRITICAL();
    switch(pin)
    {
        case OUTPUT_BEEP:
            pcf8574_write_data_.u.beep = io_status;
            break;

        case OUTPUT_DCMI_PWDN:
            pcf8574_write_data_.u.dcmi_pwdn = io_status;
            break;

        case OUTPUT_USB_PWR:
            pcf8574_write_data_.u.usb_pwr = io_status;
            break;

        case OUTPUT_RS485_SEL:
            pcf8574_write_data_.u.rs485_sel = io_status;
            break;

        case OUTPUT_ETH_RESET:
            pcf8574_write_data_.u.eth_reset = io_status;
            break;
        
        default:
            break;
    }
    portEXIT_CRITICAL();
    
    trigger(I2C_EVENT_IO_CHIP_WRITE, nullptr, 0);
}

void i2c_monitor::run(void* parameter)
{
    i2c_event event;
    
    i2c_monitor::get_instance()->registerDevUpdateTimeTrigger();
    
    while(1)
    {
        if(xQueueReceive(queue_, &event, portMAX_DELAY) == pdPASS)
        { 
            if(event.id == I2C_EVENT_IO_CHIP_WRITE)
            {
                pcf8574_i2c_write(pcf8574_write_data_.data);
                PRINT_LOG(LOG_INFO, "i2c write:0x%x!", pcf8574_write_data_.data);
            }
            else if(event.id == I2C_EVENT_IO_CHIP_READ)
            {
                uint8_t io_read;
                
                __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
                HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); 
                
                if(pcf8574_i2c_read(&io_read) == pdPASS)
                {
                    pcf8574_read_data_.data = io_read;
                    PRINT_LOG(LOG_INFO, "i2c read:0x%x!", io_read);
                }
                else
                {
                   PRINT_LOG(LOG_ERROR, "i2c read failed!");
                }
            }
            else if(event.id == I2C_EVENT_IO_DELAY_READ)
            {
               i2c_monitor::get_instance()->registerIODelayTimeTrigger();
               PRINT_LOG(LOG_ERROR, "i2c trigger delay read!");
            }
            else if(event.id == I2C_EVENT_DEVICE_UPDATE)
            {
               i2c_monitor::get_instance()->ap3216c_i2c_run();
            }
        }
    }
}

void i2c_monitor::ap3216c_i2c_run(void)
{
    uint8_t buf[6];
    uint8_t index;
    
    for(index=0; index<6; index++)
    {
        if(ap3216_i2c_multi_read(AP3216C_IRDATALOW+index, &buf[index], 1) != pdPASS)
        {
            PRINT_LOG(LOG_INFO, "ap3216c i2c device read failed.");
            return;
        }
    }

    //ir data
    if(buf[0]&(1<<7)) 	
        ap3216_info_.ir = 0;					
    else 			
        ap3216_info_.ir = (((uint16_t)buf[1])<< 2) | (buf[0]&0X03); 			
    
    ap3216_info_.als = ((uint16_t)buf[3]<<8) | buf[2];
    
    if(buf[4]&(1<<6))	
        ap3216_info_.ps = 0;    													
    else 				
        ap3216_info_.ps = ((uint16_t)(buf[5]&0X3F)<<4)|(buf[4]&0X0F); 
    
    
    PRINT_LOG(LOG_INFO, "ap3216c i2c read success, ir:%d, als:%d, ps:%d.",
        ap3216_info_.ir, ap3216_info_.als, ap3216_info_.ps);
}

void i2c_monitor::registerIODelayTimeTrigger(void)
{
    removeIODelayTimeTrigger();
    
    SysTimeManage::get_instance()->registerEventTrigger(EVENT_I2C_IO_DELAY_TIMER_TIGGER, 
                                                        TIME_MANAGE_CNT(I2C_IO_DELAY_READ_TIME_MS), 
                                                        &TriggerFunc, NULL, 1);
}

void i2c_monitor::removeIODelayTimeTrigger(void)
{
   SysTimeManage::get_instance()->removeEventTrigger(EVENT_I2C_IO_DELAY_TIMER_TIGGER);
}

void i2c_monitor::registerDevUpdateTimeTrigger(void)
{
    removeDevUpdateTimeTrigger();
    SysTimeManage::get_instance()->registerEventTrigger(EVENT_I2C_DEV_UPDATE_TIMER_TIGGER, 
                                                        TIME_MANAGE_CNT(I2C_DEVICE_UPDATE_TIME_MS), 
                                                        &TriggerFunc, NULL, TIMER_TRIGGER_FOREVER);
}

void i2c_monitor::removeDevUpdateTimeTrigger(void)
{
    SysTimeManage::get_instance()->removeEventTrigger(EVENT_I2C_DEV_UPDATE_TIMER_TIGGER);   
}

void I2cTriggerFunc::operator() (uint16_t timer_event)
{
    if(timer_event == EVENT_I2C_IO_DELAY_TIMER_TIGGER)
    {
        i2c_monitor::get_instance()->trigger(I2C_EVENT_IO_CHIP_READ, nullptr, 0);
    }
    else if(timer_event == EVENT_I2C_DEV_UPDATE_TIMER_TIGGER)
    {
        i2c_monitor::get_instance()->trigger(I2C_EVENT_DEVICE_UPDATE, nullptr, 0);
    }
}

extern "C"
{
    void i2c_isr_trigger(void)
    {
        i2c_monitor::get_instance()->trigger_isr(I2C_EVENT_IO_DELAY_READ, nullptr, 0);
    }
    
    void i2c_write_io(uint8_t pin, uint8_t status)
    {
        i2c_monitor::get_instance()->pcf8574_write_io(pin, status);
    }
}