//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      monitor.cpp
//
//  Purpose:
//      system monitor functional.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "monitor.hpp"
#include "i2c_monitor.hpp"
#include "multi_button.h"

BaseType_t monitor_manage::init()
{
    BaseType_t xReturn;
    int index;
    
    for(index=0; index<KEY_NUM; index++)
    {
        key_last_[index] = KEY_OFF;
        key_now_[index] = KEY_OFF;
        tick[index] = 0;
    }
    
    xReturn = xTaskCreate(run, "monitor_manage",        
                        MONITOR_TASK_STACK, 
                        (void *)this,   
                        MONITOR_TASK_PROITY,
                        &task_handle_ );                   
    return xReturn;
}

bool monitor_manage::is_time_escape(uint32_t ticks , uint32_t time)
{
    uint32_t tick_end = ticks + time;
    uint32_t now_tick =  xTaskGetTickCount();
    
    if(tick_end > ticks)
    {
        if(now_tick >= tick_end 
        || ((now_tick < tick_end) && ((tick_end - now_tick) > 65535)))
            return true;
    }
    else
    {
        if(now_tick >= tick_end && now_tick < ticks)
            return true;
    }
    
    return false;
}

const std::function<void()> key_func_list[] = {
    [](){
        PRINT_LOG(LOG_INFO_RECORD, "Tpad Key Push down, no_push:%d, push:%d!",
            tpad_get_no_push_val(),
            tpad_current_val()
        );
        rtc_delay_alarm(0, 0, 0, 5);
    },    
    [](){
        PRINT_LOG(LOG_INFO_RECORD, "EXIO Push down!");
    },    
};

enum Button_IDs {
	btn0_id = 0,
    btn1_id,
    btn2_id,
    btn3_id,
};

uint8_t read_button_GPIO(uint8_t button_id)
{
    uint8_t gpio_state = 0;
    
    switch(button_id)
    {
        case btn0_id:
            gpio_state = key_get_value(0);
            break;
        case btn1_id:
            gpio_state = key_get_value(1);
            break;
        case btn2_id:
            gpio_state = key_get_value(2);
            break;
        case btn3_id:
            gpio_state = ((key_get_value(0)== GPIO_PIN_SET)&&(key_get_value(1) == GPIO_PIN_SET))?GPIO_PIN_SET:GPIO_PIN_RESET;
            break;        
        default:
            break;
    }
    return gpio_state == GPIO_PIN_SET?1:0;
}

/*
按键除了按下，抬起，长按，短按等，还有多个按键的组合功能和限制
1.两个按键的功能冲突, 同时只能一个生效一个(加/减按钮)
2.组合按键，两个按键同时按一段时间，执行不同的命令
设计功能:
1.Btn0, Btn1同时按下，只能一个生效
2.Btn0，Btn1同时长按一段时间，则产生一个事件
*/
static struct Button btn0, btn1, btn2, btn3;
void BTN0_PRESS_Handler(void* btn)
{
    PressEvent event = get_button_event((struct Button*)btn);
    
    //当btn1按下时, btn0不处理
    if(is_button_active(&btn1))
        return;

    if(event == PRESS_DOWN)
    {
        PRINT_LOG(LOG_INFO_RECORD, "Key0 Push down!");
        i2c_monitor::get_instance()->pcf8574_write_io(OUTPUT_BEEP, IO_ON);
    }
    else if(event == PRESS_UP)
    {
        PRINT_LOG(LOG_INFO_RECORD, "Key0 Push up!");
        i2c_monitor::get_instance()->pcf8574_write_io(OUTPUT_BEEP, IO_OFF);            
    }
}

void BTN1_PRESS_Handler(void* btn)
{
    PressEvent event = get_button_event((struct Button*)btn);
    uint32_t buffer[] = {
        0x1234, 0x02341515, 0x12321321, 0x23458686 
    };
    
    //当btn0按下时, btn1不处理
    if(is_button_active(&btn0))
        return;

    if(event == PRESS_DOWN)
    {
        uint32_t hw_crc_value;
        uint32_t soft_crc_value;
        
        hw_crc_value = calc_hw_crc32(buffer, 4);
        soft_crc_value = calc_crc32(buffer, 4);
        
        PRINT_LOG(LOG_INFO_RECORD, "Key1 Push down!");
        PRINT_LOG(LOG_INFO_RECORD, "rng:%d crc:0x%x, 0x%x", rng_get_value(), hw_crc_value, soft_crc_value);
        PRINT_LOG(LOG_INFO_RECORD, "time per cnt:0x%x", monitor_manage::get_instance()->get_per_second());
    }
    else if(event == PRESS_UP)
    {
        PRINT_LOG(LOG_INFO_RECORD, "Key1 Push up!");    
    }  
}

void BTN2_PRESS_Handler(void* btn)
{
    PressEvent event = get_button_event((struct Button*)btn);
    
    if(event == PRESS_DOWN)
    {
        static float precent = 1;
        
        PRINT_LOG(LOG_INFO_RECORD, "Key2 Push down!");
        set_convert_vol(precent);
        pwm_set_percent(precent);
        precent -= 0.1;
        if(precent < 0.5)
            precent = 1;
       
        auto ap3216_info = i2c_monitor::get_instance()->get_ap3216_val();
        
        PRINT_LOG(LOG_INFO_RECORD, "ap3216c i2c read success, ir:%d, als:%d, ps:%d.",
            ap3216_info.ir, ap3216_info.als, ap3216_info.ps);
    }
    else if(event == PRESS_UP)
    {
        PRINT_LOG(LOG_INFO_RECORD, "Key2 Push up!");    
    }  
}

void BTN3_LONG_PRESS_Handler(void *btn)
{
    PRINT_LOG(LOG_INFO_RECORD, "long press group button set!");    
}

void monitor_manage::key_motion()
{
    static uint8_t key_first_run = 0;
    
    if(key_first_run == 0)
    {
        key_first_run = 1;
        
        button_init(&btn0, read_button_GPIO, 0, btn0_id);
        button_init(&btn1, read_button_GPIO, 0, btn1_id);
        button_init(&btn2, read_button_GPIO, 0, btn2_id);
        button_init(&btn3, read_button_GPIO, 0, btn3_id);

        button_attach(&btn0, PRESS_DOWN,  BTN0_PRESS_Handler);
        button_attach(&btn0, PRESS_UP,  BTN0_PRESS_Handler);
        button_attach(&btn1, PRESS_DOWN,  BTN1_PRESS_Handler);
        button_attach(&btn1, PRESS_UP,  BTN1_PRESS_Handler);
        button_attach(&btn2, PRESS_DOWN,  BTN2_PRESS_Handler);
        button_attach(&btn2, PRESS_UP,  BTN2_PRESS_Handler);
        button_attach(&btn3, LONG_PRESS_START,  BTN3_LONG_PRESS_Handler);

        button_start(&btn0);
        button_start(&btn1);
        button_start(&btn2);
    }
    
    button_ticks();
    
    key_now_[0] = monitor_manage::get_instance()->anti_shake(&tick[0], key_now_[0], tpad_scan_key()==1?KEY_ON:KEY_OFF);
    key_now_[1] = monitor_manage::get_instance()->anti_shake(&tick[1], key_now_[1], i2c_monitor::get_instance()->get_read_io()->u.exio==0?KEY_ON:KEY_OFF);
    
    for(int index=0; index<KEY_NUM; index++)
    {
        if(key_now_[index] == KEY_ON)
        {
            if(key_last_[index] != key_now_[index])
            {
                key_last_[index] = key_now_[index];
                key_func_list[index]();
            }
        }
        else
        {
            key_last_[index] = key_now_[index];
        }
    }
}

void monitor_manage::timer_loop_motion()
{
    static uint8_t last_second = 0;
    static uint8_t temp_loop = 0;
    char tbuf[60];
    
     temp_loop++;
     if(temp_loop >= 10)
     {
        temp_loop = 0;
        
        auto rtc_info = rtc_update();
        if(last_second != rtc_info.time.Seconds)
        {
            last_second = rtc_info.time.Seconds;
            sprintf(tbuf, "Timer: %02d-%02d-%02d %02d:%02d:%02d",
                rtc_info.date.Year,
                rtc_info.date.Month,
                rtc_info.date.Date,
                rtc_info.time.Hours,
                rtc_info.time.Minutes,
                rtc_info.time.Seconds);
            lcd_driver_showstring(10, 160, 200, 16, 16, tbuf);
        }
    }
     
    if(rtc_get_alarm_flag() == pdTRUE)
    {
        PRINT_LOG(LOG_INFO_RECORD, "RTC Alarm");

        rtc_set_alarm_flag(pdFALSE);
    }
}

void monitor_manage::adc_monitor()
{
    static uint8_t temp_loop = 0;
    static uint32_t adc_temp = 0, adc_vol;
    double temperate;
    double voltage;

    temp_loop++;
    if(temp_loop >= 20)
    {
        temp_loop = 0;

        adc_temp = adc_get_avg(ADC_CHANNEL_TEMPSENSOR);
        temperate = (float)adc_temp*(3.3/4096);	
        temperate = (temperate-0.76)/0.0025 + 25; 

        lcd_driver_show_extra_num(10+11*8,140,(uint32_t)temperate, 2, 16, 0);		
        lcd_driver_show_extra_num(10+14*8,140,((uint32_t)(temperate*100))%100, 2, 16, 0);		
        
        adc_vol = adc_get_avg(ADC_CHANNEL_6);
        voltage = (float)adc_vol*(3.3/4096);
        lcd_driver_show_extra_num(10+23*8,140,(uint32_t)voltage, 2, 16, 0);	
        lcd_driver_show_extra_num(10+26*8,140,((uint32_t)(voltage*100))%100, 2, 16, 0);
    }
}

void monitor_manage::task_per_second(void)
{
    static uint16_t temp_loop = 0;
    
    temp_loop++;
    if(temp_loop >= 50)
    {
        temp_loop = 0;
        per_second = get_cnt_per_second(); 
    }
}


void monitor_manage::run(void* parameter)
{
    monitor_manage* pInstance = (monitor_manage *)parameter;
    
    logger_set_multi_thread_run();
    
    while(1)
    {
        pInstance->timer_loop_motion();
        
        //key motion loop
        pInstance->key_motion();
         
        pInstance->adc_monitor();
        
        pInstance->task_per_second();
        
        iwdg_reload();
        
        vTaskDelay(20);
    }
}

KEY_STATE monitor_manage::anti_shake(uint8_t *pTick, KEY_STATE nowIoStatus, KEY_STATE readIoStatus)
{
    GPIO_PinState OutIoStatus = nowIoStatus;

    (*pTick) += 1;

    if(nowIoStatus == KEY_ON)
    {
        if(readIoStatus == KEY_OFF)
        {
            if(*pTick > ANTI_SHAKE_TICK)
            {
                OutIoStatus = KEY_OFF;
                *pTick = 0;
            }
        }
        else
        {
            *pTick = KEY_ON;
        }
    }
    else if(nowIoStatus == KEY_OFF)
    {
        if(readIoStatus == KEY_ON)
        {
            if(*pTick > ANTI_SHAKE_TICK)
            {
                OutIoStatus = KEY_ON;
                *pTick = 0;
            }
        }
        else
        {
            *pTick = KEY_OFF;
        }
    }

    return OutIoStatus;    
}