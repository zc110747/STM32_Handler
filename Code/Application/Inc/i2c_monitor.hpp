//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      i2c_monitor.hpp
//
//  Purpose:
//      i2c monitor module, support process all i2c deivce extend.
//      now support pcf8574, ap3216, mpu9250
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "driver.h"
#include "timer_manage.hpp"

#define i2c_monitor_MAX_QUEUE               32

#define I2C_EVENT_IO_CHIP_WRITE             0       //write i2c io extern
#define I2C_EVENT_IO_CHIP_READ              1       //read i2c io extern
#define I2C_EVENT_IO_DELAY_READ             2
#define I2C_EVENT_DEVICE_REFRESH            3       //update the i2c device info

#define I2C_IO_DELAY_READ_TIME_MS           pdMS_TO_TICKS(500)
#define I2C_DEVICE_UPDATE_TIME_MS           pdMS_TO_TICKS(1000)

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

#if I2C_MONITOR_MODULE_STATE  == MODULE_ON
class i2c_monitor
{
public: 
    /// \brief constructor.    
    i2c_monitor() {}
        
    /// \brief destructor.    
    virtual ~i2c_monitor() {}
        
    /// \brief initalize
    /// - This method is initialize the i2c_monitor task.        
    BaseType_t init();

    /// \brief get_instance
    /// - This method is used to get the pattern of the class.
    /// \return the singleton pattern point of the object.        
    static i2c_monitor* get_instance()
    {
        static i2c_monitor instance_;
        return &instance_;
    }

    /// \brief pcf8574_write_io
    /// - This method is set io status for the i2c i/o extend chip.
    /// \param id           - id of the trigger event.    
    void pcf8574_write_io(uint8_t pin, uint8_t status);
    
    /// \brief trigger
    /// - This method is trigger event with buffer to i2c task.
    /// \param event        - event tell the i2c.     
    /// \param pdata        - buffer with the event. 
    /// \param size         - buffer size. 
    BaseType_t trigger(uint8_t event, uint8_t *pdata, uint8_t size);
    
    /// \brief trigger_isr
    /// - This method is trigger event with buffer for interrupt to i2c task.
    /// \param event        - event tell the i2c.     
    /// \param pdata        - buffer with the event. 
    /// \param size         - buffer size.     
    BaseType_t trigger_isr(uint8_t event, uint8_t *pdata, uint8_t size);
    
    /// \brief get_queue
    /// - get the point of i2c task wait queue.
    /// \return the point of the i2c task wait queue.
    QueueHandle_t get_queue(void)   {return queue_;}

    /// \brief get_queue
    /// - get the point of write I/O data.
    /// \return the point of the i2c write I/O data . 
    PC8574_IO *get_write_io()       {return &pcf8574_write_data_;}    

    /// \brief get_read_io
    /// - get the point of read I/O data.
    /// \return the point of the i2c read I/O data.      
    PC8574_IO *get_read_io()        {return &pcf8574_read_data_;}   

    /// \brief get_read_io
    /// - get the point of ap3216 data.
    /// \return the point of the i2c read I/O buffer       
    AP3216C_INFO get_ap3216_val()   {return ap3216_info_;}
        
private:
    /// \brief run
    /// - task for the i2c_montion run.
    /// \param parameter    - parameter translate when create.  
    static void run(void* parameter);
    
    /// \brief registerDevUpdateTimeTrigger
    /// - register device update peroid trigger.
    void registerDevUpdateTimeTrigger(void);

    /// \brief removeDevUpdateTimeTrigger
    /// - remove device update peroid trigger.
	void removeDevUpdateTimeTrigger(void);

    /// \brief registerIODelayTimeTrigger
    /// - register i/o delay read once trigger.
	void registerIODelayTimeTrigger(void);

    /// \brief registerIODelayTimeTrigger
    /// - remove i/o delay read once trigger.
	void removeIODelayTimeTrigger(void);

    /// \brief ap3216c_i2c_run
    /// - function for i2c loop run check.
    void ap3216c_i2c_run(void);

private:
    /// \brief task_handle_
    /// - task_handle_ for the i2c monitor task.
    TaskHandle_t task_handle_{nullptr};

    /// \brief queue_
    /// - queue for i2c monitor task process.    
    QueueHandle_t queue_{nullptr};

    /// \brief pcf8574_read_data_
    /// - i/o extend chip write data.        
    PC8574_IO pcf8574_read_data_{0xff};

    /// \brief pcf8574_read_data_
    /// - i/o extend chip read data. 
    PC8574_IO pcf8574_write_data_{0xff};

    /// \brief ap3216_info_
    /// - ap3216 read data.     
    AP3216C_INFO ap3216_info_;

    /// \brief TriggerFunc
    /// - trigger func for i2c callback.     
    I2cTriggerFunc TriggerFunc;
};
#else
class i2c_monitor
{
public: 
    /// \brief constructor.    
    i2c_monitor() {}
        
    /// \brief destructor.    
    virtual ~i2c_monitor() {}
    
    /// \brief initalize
    /// - This method is initialize the i2c_monitor task.        
    BaseType_t init()   {
        return pdPASS;
    }

    /// \brief get_instance
    /// - This method is used to get the pattern of the class.
    /// \return the singleton pattern point of the object.        
    static i2c_monitor* get_instance()
    {
        static i2c_monitor instance_;
        return &instance_;
    }

    /// \brief pcf8574_write_io
    /// - This method is set io status for the i2c i/o extend chip.
    /// \param id           - id of the trigger event.    
    void pcf8574_write_io(uint8_t pin, uint8_t status){
    }

    /// \brief get_read_io
    /// - get the point of read I/O data.
    /// \return the point of the i2c read I/O data.      
    PC8574_IO *get_read_io()        {return &pcf8574_read_data_;}   

    /// \brief get_read_io
    /// - get the point of ap3216 data.
    /// \return the point of the i2c read I/O buffer       
    AP3216C_INFO get_ap3216_val()   {return ap3216_info_;}
    
private:
    /// \brief ap3216_info_
    /// - ap3216 read data.     
    AP3216C_INFO ap3216_info_;

    /// \brief pcf8574_read_data_
    /// - i/o extend chip write data.        
    PC8574_IO pcf8574_read_data_{0xff};
};
#endif
