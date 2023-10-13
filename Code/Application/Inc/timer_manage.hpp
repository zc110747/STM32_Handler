//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      timer_manage.hpp
//
//  Purpose:
//      timer manage module, support period run and clock count.
//		support a trigger list can delay or loop run task.
//
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _TIMER_MANAGE_H
#define _TIMER_MANAGE_H

#include "main.h"

#define EVENT_I2C_IO_DELAY_TIMER_TIGGER         0
#define EVENT_I2C_DEV_UPDATE_TIMER_TIGGER       1
#define SYSTEM_TIME_MAX_TRIGGERS	            10
#define SOFT_LOOP_TIMER_PERIOD		            (pdMS_TO_TICKS(100))
#define TIME_MANAGE_CNT(T)                      ((T)/SOFT_LOOP_TIMER_PERIOD)
#define TIMER_TRIGGER_FOREVER                   UINT16_MAX

template <typename T>
class TimeTriggerFunction
{
public:
	virtual ~TimeTriggerFunction(){}
	virtual void operator()(uint16_t id, T d){}
	virtual void operator()(uint16_t id){}
};

template<class T>
class TimeTrigger
{
private:
    /// \brief triggerFunc_
    /// - point of trigger action callback.   
	TimeTriggerFunction<uint32_t>* triggerFunc_;

    /// \brief time_compare_
    /// - timer compare delay for trigger action.  
	uint32_t time_compare_;

    /// \brief time_cmp_count_
    /// - timer compare count for trigger action.  
	uint32_t time_cmp_count_;

    /// \brief trigger_var_
    /// - trigger var for callback.  
	T *trigger_var_;

private:
    /// \brief isMatch
    /// - This method is detect wheater trigger match action.    
	bool isMatch()
    {
		time_cmp_count_++;
		if(time_cmp_count_ >= time_compare_)
		{
			time_cmp_count_ = 0;
			return true;
		}

        return false;
	}

public:  
    /// \brief instance_pointer_
    /// - trigger already used state.
    uint8_t used_;

    /// \brief triggerId_
    /// - id used to differentiated trigger .
    uint16_t triggerId_;  

    /// \brief triggerCount_
    /// - count of trigger can action.
    uint16_t triggerCount_;  

    /// \brief getId
    /// - interface get trigger id.          
    uint16_t getId()        {return triggerId_;}
    
    /// \brief run
    /// - interface get the trigger can action times.
    uint16_t getCount()     {return triggerCount_;}

public:
    
    /// \brief constructor.
    TimeTrigger(){}
    
    /// \brief destructor.
    virtual ~TimeTrigger(){}
    
    /// \brief initalize
    /// - This method is initialize the trigger information.
    /// \param id           - id of the trigger event.
    /// \param time_cnt     - time compare count due to period.
    /// \param pv           - value translate to action, not local variable.
    /// \param TriggerCount - trigger can run times.
    /// \param func         - function for callback action.        
	void initalize(uint16_t id, uint32_t time_cnt, T* pv, uint16_t TriggerCount, TimeTriggerFunction<uint32_t>* func)
	{
		triggerId_ = id;
		time_compare_ = time_cnt;
		trigger_var_ = pv;
		triggerFunc_ = func;
		triggerCount_ = TriggerCount;
        time_cmp_count_ = 0;
	}

    /// \brief action
    /// - This method is the trigger action.        
	void action(void)
	{
		if(triggerCount_>0 && isMatch())
		{
			if(triggerCount_ != TIMER_TRIGGER_FOREVER)
			{
				--triggerCount_;
			}
			if(NULL != triggerFunc_)
			{
				if(trigger_var_ != NULL) 
					(*triggerFunc_)(triggerId_, *trigger_var_);
				else
					(*triggerFunc_)(triggerId_);
			}
		}
	}
};

class SysTimeManage
{
private:
    
     /// \brief Instance
    /// - object used to implement the singleton pattern.
	static SysTimeManage Instance; 

    /// \brief timeCount_
    /// - object used to implement the singleton pattern.
	volatile uint32_t timeCount_{0};
    
    /// \brief SysTimeHandler_
    /// - object used to save the freertos timer handle. 
	TimerHandle_t SysTimeHandler_{NULL};
    
    /// \brief SysTimeSemaphore_
    /// - semaphore used to protect time handle. 
	SemaphoreHandle_t SysTimeSemaphore_{NULL};
    
    /// \brief triggerList_
    /// - Array of List save all trigger object. 
	TimeTrigger<uint32_t> triggerList_[SYSTEM_TIME_MAX_TRIGGERS];
    
public:
    /// \brief constructor.
	SysTimeManage() {}

    /// \brief destructor.
	virtual ~SysTimeManage() {}

    /// \brief get_instance
    /// - This method is used to get the pattern of the class.
    /// \return the singleton pattern point of the object.
	static SysTimeManage *get_instance();
        
    /// \brief initalize
    /// - This method is initialize the time manage.
	BaseType_t init();

    /// \brief updateTimeCount
    /// - This method is update the time count in timer.
    void updateTimeCount()  {timeCount_++;}
    
    /// \brief getTimeCount
    /// - This method is get the current time count.
    /// \return return the current time count
    uint32_t getTimeCount()     {return timeCount_;}

    /// \brief processTrigger
    /// - process the trigger also used in array.
    void processTrigger();

    /// \brief removeEventTrigger
    /// - remove the trigger accord the event id.
    void removeEventTrigger(uint16_t id);

    /// \brief initalize
    /// - This method is register a new trigger in array.
    /// \param id           - id of the trigger event.
    /// \param time_cnt     - time compare count due to period.
    /// \param func         - function for callback action.  
    /// \param pv           - value translate to action, not local variable.
    /// \param TriggerCount - trigger can run times.
    /// \return register success(1) or not(0) 
    uint32_t registerEventTrigger(uint16_t id, 
                                uint32_t time_cnt, 
                                TimeTriggerFunction<uint32_t>* func,
    							uint32_t *pv,
                                uint16_t TriggerCount=1);
};

#endif