#pragma once

#include "driver.h"
#include "includes.hpp"
#include "timer_manage.hpp"

#define i2c_monitor_MAX_QUEUE               32

#define I2C_EVENT_IO_CHIP_WRITE             0       //write i2c io extern
#define I2C_EVENT_IO_CHIP_READ              1       //read i2c io extern
#define I2C_EVENT_IO_DELAY_READ             2
#define I2C_EVENT_DEVICE_UPDATE             3       //update the i2c device info

#define I2C_IO_DELAY_READ_TIME_MS           pdMS_TO_TICKS(500)
#define I2C_DEVICE_UPDATE_TIME_MS           pdMS_TO_TICKS(200)

typedef union
{
    uint8_t data;
    
    struct
    {
        uint8_t beep:1;
        uint8_t ap_int:1;
        uint8_t dcmi_pwdn:1;
        uint8_t usb_pwr:1;
        uint8_t exio:1;
        uint8_t d_int:1;
        uint8_t rs485_sel:1;
        uint8_t eth_reset:1;
    }u;
}PC8574_IO;

typedef struct
{
    uint8_t id;
    uint8_t data;
}i2c_event;

typedef struct
{
    uint16_t ir;
    uint16_t als;
    uint16_t ps;
}AP3216C_INFO;

class I2cTriggerFunc: public TimeTriggerFunction<uint32_t>
{
public:
	void operator() (uint16_t timer_event);
};

class i2c_monitor
{
public:  
    BaseType_t init();
    
    static i2c_monitor* get_instance()
    {
        static i2c_monitor instance_;
        return &instance_;
    }
   
    void pcf8574_write_io(uint8_t pin, uint8_t status);
    
    BaseType_t trigger(uint8_t event, uint8_t *pdata, uint8_t size);
    BaseType_t trigger_isr(uint8_t event, uint8_t *pdata, uint8_t size);
    
public:
    QueueHandle_t get_queue(void)   {return queue_;}
    
    PC8574_IO *get_write_io() {
        return &pcf8574_write_data_;
    }    
    
    PC8574_IO *get_read_io() {
        return &pcf8574_read_data_;
    }   
    
    AP3216C_INFO get_ap3216_val(){
        return ap3216_info_;
    }
        
private:
    static void run(void* parameter);
    
    void registerDevUpdateTimeTrigger(void);
	void removeDevUpdateTimeTrigger(void);
    
	void registerIODelayTimeTrigger(void);
	void removeIODelayTimeTrigger(void);
    
    void ap3216c_i2c_run(void);

private:
    TaskHandle_t task_handle_{nullptr};
    
    QueueHandle_t queue_{nullptr};
    
    PC8574_IO pcf8574_read_data_{0xff};

    PC8574_IO pcf8574_write_data_{0xff};
    
    AP3216C_INFO ap3216_info_;

    I2cTriggerFunc TriggerFunc;
};
