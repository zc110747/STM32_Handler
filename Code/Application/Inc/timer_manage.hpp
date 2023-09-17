//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//     timer_manage.hpp
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
#ifndef _TIMER_MANAGE_H
#define _TIMER_MANAGE_H

#include "main.h"
#include <new>

#define EVENT_I2C_TIMER_TIGGER      0

#define SYSTEM_TIME_MAX_TRIGGERS	10
#define SOFT_LOOP_TIMER_PERIOD		(pdMS_TO_TICKS(100))

template <typename T>
class TimeTriggerFunction
{
public:
	virtual ~TimeTriggerFunction(){}
	virtual void operator()(T d){}
	virtual void operator()(void){}
};

class UserMemoryMange
{
public:
	UserMemoryMange(){}
		~UserMemoryMange(){}

public:
	static void *operator new(size_t size){
		void *pT = static_cast<void *>(pvPortMalloc(size));

		return pT;
	}

	static void *operator new(size_t size, const uint16_t& id)
	{
		void *pT = static_cast<void *>(pvPortMalloc(size));

		return pT;
	}

	static void *operator new(size_t size, const std::nothrow_t&){
		void *pT = static_cast<void *>(pvPortMalloc(size));
        
		return pT;
	}

	static void operator delete(void *ptr){
		if(ptr != NULL)
		{
			vPortFree(ptr);
		}
	}

	static void *operator new[](size_t size, const std::nothrow_t&)
	{
		void *pT = static_cast<void *>(pvPortMalloc(size));
		return pT;
	}

	static void operator delete[](void *pT)
	{
		if(pT != NULL)
		{
			vPortFree(pT);
		}
	}
};

template<class T>
class TimeTrigger:public UserMemoryMange
{
private:
	TimeTriggerFunction<uint32_t>* pFunc; //callback function when trigger matched
	uint32_t compare;
	uint32_t CmpCount;
	T *pVar;

private:
	bool isMatch(){
		CmpCount++;

		if(CmpCount >= compare)
		{
			CmpCount = 0;
			return true;
		}
		else
		{
			return false;
		}
	}

public:
    uint16_t trigId;  // id for the event
    uint16_t triggerCount;  // init by user, -1 every trigger

    virtual ~TimeTrigger(){}
    uint16_t getId(){return trigId;}
    uint16_t getCount(){return triggerCount;}

public:
    TimeTrigger(uint16_t id, uint32_t c, T* pv,uint16_t trigCount, TimeTriggerFunction<uint32_t>* f)
	{
		trigId = id;
		compare = c;
		pVar = pv;
		pFunc = f;
		triggerCount = trigCount;
		CmpCount = 0;
	}

	void TriggerUpdate(uint16_t id, uint32_t c, T* pv,uint16_t trigCount, TimeTriggerFunction<uint32_t>* f)
	{
		trigId = id;
		compare = c;
		pVar = pv;
		pFunc = f;
		triggerCount = trigCount;
	}

	void action(void)
	{
		if(triggerCount>0 && isMatch())
		{
			if(triggerCount != UINT16_MAX)
			{
				--triggerCount;
			}
			if(NULL != pFunc)
			{
				if(pVar != NULL) //if no pVar, just send nothing
					(*pFunc)(*pVar);
				else
					(*pFunc)();
			}
		}
	}
};


class SysTimeManage
{
private:
	volatile uint64_t timeCountM{0};
	TimerHandle_t timeHandleM{NULL};
	SemaphoreHandle_t xSemaphoreM{NULL};
	TimeTrigger<uint32_t>* triggerListM[SYSTEM_TIME_MAX_TRIGGERS]{NULL};

private:
    uint32_t registerTrigger(TimeTrigger<uint32_t>* pEt);
    void removeTrigger(uint16_t id);

public:
	static SysTimeManage Instance;
	static SysTimeManage *get_instance();

public:
	SysTimeManage();
		~SysTimeManage();

	BaseType_t init();
	BaseType_t start();

	uint8_t is_timer_elapsed(uint32_t time, uint32_t duration);
	void timeCountUpdate(void);
    void processTrigger();

    void removeEventTrigger(uint16_t id){
    	removeTrigger(id);
    }
    uint32_t registerEventTrigger(uint16_t id, uint32_t cmp, TimeTriggerFunction<uint32_t>* f,
    							uint32_t *pVar, uint16_t trigcount=1);
public:
	uint32_t getTimeCount() {return timeCountM;};
};

#endif