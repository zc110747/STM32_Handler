
#include "monitor.hpp"
#include "lcd.hpp"
#include "driver.hpp"
#include "i2c_monitor.hpp"

KEY_STATE monitor_manage::key_last_[KEY_NUM];
KEY_STATE monitor_manage::key_now_[KEY_NUM];
uint8_t monitor_manage::tick[KEY_NUM];

bool monitor_manage::init()
{
    BaseType_t xReturned;
    int index;
    
    for(index=0; index<KEY_NUM; index++)
    {
        key_last_[index] = KEY_OFF;
        key_now_[index] = KEY_OFF;
        tick[index] = 0;
    }
    
    xReturned = xTaskCreate(
                    run,      
                    "monitor_manage",        
                    MONITOR_TASK_STACK,              
                    ( void * ) NULL,   
                    MONITOR_TASK_PROITY,
                    &task_handle_ );      
  
   if(xReturned == pdPASS)
       return true;
   return false;
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

uint32_t buffer[] = {
    0x1234, 0x02341515, 0x12321321, 0x23458686 
};
std::function<void()> key_func_list[] = {
    [](){
        PRINT_LOG(LOG_INFO, "Key0 Push down!");
        i2c_monitor::get_instance()->pcf8574_write_io(OUTPUT_BEEP, IO_ON);
    },
    [](){
        uint32_t crc_value;
       
        crc_value = crc_get_value(buffer, 4);       
        PRINT_LOG(LOG_INFO, "Key1 Push down!");
        i2c_monitor::get_instance()->pcf8574_write_io(OUTPUT_BEEP, IO_OFF);
        PRINT_LOG(LOG_INFO, "rng:%d crc:0x%x", rng_get_value(), crc_value);
    },
    [](){
        static float precent = 1;
        PRINT_LOG(LOG_INFO, "Key2 Push down!");
        set_convert_vol(precent);
        pwm_set_percent(precent);
        precent -= 0.1;
        if(precent < 0.5)
            precent = 1;
    },
    [](){
        PRINT_LOG(LOG_INFO, "Tpad Key Push down, no_push:%d, push:%d!",
            tpad_get_no_push_val(),
            tpad_current_val()
        );
        rtc_delay_alarm(0, 0, 0, 5);
    },    
    [](){
        PRINT_LOG(LOG_INFO, "EXIO Push down!");
    },    
};

void monitor_manage::key_motion()
{
    key_now_[0] = monitor_manage::get_instance()->anti_shake(&tick[0], key_now_[0], key_get_value(0));
    key_now_[1] = monitor_manage::get_instance()->anti_shake(&tick[1], key_now_[1], key_get_value(1));
    key_now_[2] = monitor_manage::get_instance()->anti_shake(&tick[2], key_now_[2], key_get_value(2));
    key_now_[3] = monitor_manage::get_instance()->anti_shake(&tick[3], key_now_[3], tpad_scan_key()==1?KEY_ON:KEY_OFF);
    key_now_[4] = monitor_manage::get_instance()->anti_shake(&tick[4], key_now_[4], i2c_monitor::get_instance()->get_read_io()->u.exio==0?KEY_ON:KEY_OFF);
    
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
            lcd_driver::get_instance()->lcd_showstring(10, 160, 200, 16, 16, tbuf);
        }
    }
     
    if(rtc_get_alarm_flag() == pdTRUE)
    {
        PRINT_LOG(LOG_INFO, "RTC Alarm");

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

        lcd_driver::get_instance()->lcd_show_extra_num(10+11*8,140,(uint32_t)temperate, 2, 16, 0);		
        lcd_driver::get_instance()->lcd_show_extra_num(10+14*8,140,((uint32_t)(temperate*100))%100, 2, 16, 0);		
        
        adc_vol = adc_get_avg(ADC_CHANNEL_6);
        voltage = (float)adc_vol*(3.3/4096);
        lcd_driver::get_instance()->lcd_show_extra_num(10+23*8,140,(uint32_t)voltage, 2, 16, 0);	
        lcd_driver::get_instance()->lcd_show_extra_num(10+26*8,140,((uint32_t)(voltage*100))%100, 2, 16, 0);
    }
}

void monitor_manage::run(void* parameter)
{
    logger_set_multi_thread_run();
    
    while(1)
    {
        timer_loop_motion();
        
        //key motion loop
        key_motion();
         
        adc_monitor();
        
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