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
#include "timer_manage.hpp"

//sys timer manage
static void prvxSoftLoopTimerCallback(TimerHandle_t xTimer)
{
	SysTimeManage *pSTM = static_cast<SysTimeManage *>(pvTimerGetTimerID(xTimer));

	//timer update
	pSTM->timeCountUpdate();

	//trigger process
	pSTM->processTrigger();
}

SysTimeManage SysTimeManage::Instance;
SysTimeManage::SysTimeManage()
{
}

SysTimeManage::~SysTimeManage()
{
}

SysTimeManage *SysTimeManage::get_instance()
{
	return &Instance;
}

void SysTimeManage::timeCountUpdate(void)
{
	timeCountM++;
}

BaseType_t SysTimeManage::init(void)
{
	timeHandleM = xTimerCreate("SysLoopTime",
								SOFT_LOOP_TIMER_PERIOD,
								pdTRUE,
								(void *)this,
								prvxSoftLoopTimerCallback);
	if(timeHandleM == NULL)
		return pdFAIL;

	xSemaphoreM = xSemaphoreCreateMutex();
	if(xSemaphoreM == NULL)
		return pdFAIL;

	return xTimerStart(timeHandleM, portMAX_DELAY);
}

BaseType_t SysTimeManage::start(void)
{
	return xTimerStart(timeHandleM, portMAX_DELAY);
}

uint8_t SysTimeManage::is_timer_elapsed(uint32_t count, uint32_t duration)
{
	uint32_t timer_count;
	uint32_t temp = count + duration;
	timer_count = getTimeCount();

	if(temp > count)
	{
		if(timer_count>=temp || ((timer_count < temp) && (temp - timer_count) > 65536))
			return 1;
	}
	else
	{
		return ((timer_count < count) && (timer_count >= temp));
	}
	return 0;
}

void SysTimeManage::processTrigger()
{
	if(pdTRUE == xSemaphoreTake(xSemaphoreM, 0))
	{
		for(int i = 0; i<SYSTEM_TIME_MAX_TRIGGERS; ++i)
		{
			if(triggerListM[i] != NULL)
			{
				triggerListM[i]->action();
			}
		}
		xSemaphoreGive(xSemaphoreM);
	}
}

uint32_t SysTimeManage::registerTrigger(TimeTrigger<uint32_t> *pEt)
{
	uint32_t res = 1;
	if(pdTRUE == xSemaphoreTake(xSemaphoreM, portMAX_DELAY))
	{
		for(int i = 0; i<SYSTEM_TIME_MAX_TRIGGERS; ++i)
		{
			//delete trigger which no need process
			if(triggerListM[i] != NULL && triggerListM[i]->getCount() == 0)
			{
				delete triggerListM[i];
				triggerListM[i] = NULL;
			}

			//register trigger
			if(triggerListM[i] == NULL)
			{

				triggerListM[i] = pEt;
				res = 0;
				break;
			}
		}
		xSemaphoreGive(xSemaphoreM);
	}

	return res;
}

void SysTimeManage::removeTrigger(uint16_t id)
{
	if(pdTRUE == xSemaphoreTake(xSemaphoreM, portMAX_DELAY))
	{
		for(int i=0; i<SYSTEM_TIME_MAX_TRIGGERS; ++i)
		{
			if(triggerListM[i] != NULL)
			{
				if(triggerListM[i]->getId() == id)
				{
					delete triggerListM[i];

					triggerListM[i] = NULL;
				}
			}
		}

		xSemaphoreGive(xSemaphoreM);
	}
}

uint32_t SysTimeManage::registerEventTrigger(uint16_t id, uint32_t cmp, TimeTriggerFunction<uint32_t>* f,
											uint32_t *pVar, uint16_t trigcount)
{
	uint32_t res  =1;
	TimeTrigger<uint32_t>* pEt = NULL;

	pEt = new(id) TimeTrigger<uint32_t>(id, cmp, pVar, trigcount, f);

	if(pEt == NULL)
	{
		return 2;
	}
	res =  registerTrigger(pEt);
	if(res)
	{
		delete pEt;
	}
	return res;
}
