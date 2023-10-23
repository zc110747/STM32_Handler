//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      timer_manage.cpp
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
#include "timer_manage.hpp"
#include "driver.h"

#if TIMER_MANAGE_MODULE_STATE == MODULE_ON
//sys timer manage
static void prvxSoftLoopTimerCallback(TimerHandle_t xTimer)
{
	SysTimeManage *pSTM = static_cast<SysTimeManage *>(pvTimerGetTimerID(xTimer));

	//timer update
	pSTM->updateTimeCount();

	//trigger process
	pSTM->processTrigger();
}

SysTimeManage SysTimeManage::Instance;
SysTimeManage *SysTimeManage::get_instance()
{
	return &Instance;
}

BaseType_t SysTimeManage::init(void)
{
    uint8_t index;
    BaseType_t xReturn;
    
    for(index = 0; index<SYSTEM_TIME_MAX_TRIGGERS; index++)
    {
        triggerList_[index].used_ = 0;
    }
    
	SysTimeHandler_ = xTimerCreate("SysLoopTime",
								SOFT_LOOP_TIMER_PERIOD,
								pdTRUE,
								(void *)this,
								prvxSoftLoopTimerCallback);
    SysTimeSemaphore_ = xSemaphoreCreateMutex();
                                
    if(SysTimeHandler_ == NULL || SysTimeSemaphore_ == NULL)
		xReturn = pdFAIL;
   
    if(xReturn != pdFAIL)
        xReturn = xTimerStart(SysTimeHandler_, portMAX_DELAY);
   
    if(xReturn == pdFAIL)
    {
        PRINT_LOG(LOG_ERROR, "SysTimeManage init failed, need check!");
    }        
    
	return xReturn;
}


void SysTimeManage::processTrigger()
{
	if(pdTRUE == xSemaphoreTake(SysTimeSemaphore_, 0))
	{
		for(int i = 0; i<SYSTEM_TIME_MAX_TRIGGERS; ++i)
		{
            //delete trigger no count
            if(triggerList_[i].used_ != 1 
             && triggerList_[i].getCount() == 0)
            {
                triggerList_[i].used_ = 0;
            }
            
            //run action
			if(triggerList_[i].used_ != 0)
			{
				triggerList_[i].action();
			}
		}
		xSemaphoreGive(SysTimeSemaphore_);
	}
}

void SysTimeManage::removeEventTrigger(uint16_t id)
{
	if(pdTRUE == xSemaphoreTake(SysTimeSemaphore_, portMAX_DELAY))
	{
		for(int i=0; i<SYSTEM_TIME_MAX_TRIGGERS; ++i)
		{  
            if(triggerList_[i].used_ != 0 
            && triggerList_[i].getId() == id)
            {
                triggerList_[i].used_ = 0;
            }
		}
		xSemaphoreGive(SysTimeSemaphore_);
	}
}

GlobalType_t SysTimeManage::registerEventTrigger(uint16_t id, uint32_t time_cnt, TimeTriggerFunction<uint32_t>* func,
											uint32_t *pv, uint16_t TriggerCount)
{  
    GlobalType_t result = GLOBAL_ERROR;
    
    if(pdTRUE == xSemaphoreTake(SysTimeSemaphore_, portMAX_DELAY))
    {
        for(int i = 0; i<SYSTEM_TIME_MAX_TRIGGERS; ++i)
        {
            //delete trigger which no need process
            if(triggerList_[i].used_ != 1 
             && triggerList_[i].getCount() == 0)
            {
                triggerList_[i].used_ = 0;
            }

            //register trigger
            if(triggerList_[i].used_ == 0)
            {
                triggerList_[i].used_ = 1;
                triggerList_[i].initalize(id, time_cnt, pv, TriggerCount, func);
                result = GLOBAL_OK;
                break;
            }
        }
        xSemaphoreGive(SysTimeSemaphore_);
    }
    
    return result;
}
#endif
