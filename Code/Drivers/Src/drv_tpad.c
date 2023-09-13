//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      tpad.cpp
//
//  Purpose:
//      use timer capture for tpad.
//      Input: PA5
//      Module: TIM2
// Author:
//      @zc
//
//  Assumptions:
//
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "drv_tpad.h"

//global define 
#define TPAD_ARR_MAX_VAL                    0XFFFFFFFF
#define TPAD_TIMER_PRESCALER                1               //clock div 2
#define TPAD_TIMES_CAPTURE_LOW              50
#define TPAD_TIMES_CAPTURE_HIGH             2500
#define TPAD_INIT_CHECK_TIMES               10
#define TPAD_CAPTURE_LOOP_TIMES             4
#define TPAD_IS_VALID_PUSH_KEY(value, no_push_value) \
    (((value)>((no_push_value)*4/3)) && ((value)<((no_push_value)*10)))
#define TPAD_IS_VALID_CAPTURE(value) \
   (((value)>=TPAD_TIMES_CAPTURE_LOW) && ((value)<=(TPAD_TIMES_CAPTURE_HIGH)))

//global parameter
static TIM_HandleTypeDef htim2_tpad = {0};
static uint16_t no_push_value_ = 0;
static uint16_t current_value_ = 0;
/*
APB2 Timer Clock 90Mhz
Timer Ticks for one Count is 45MHZ
*/

//function
static void tpad_reset(void);
static BaseType_t tpad_hardware_init(void);
static uint16_t tpad_get_value(void);
static uint16_t tpad_get_max_value(void);

int comp(const void *a,const void* b)
{
    return *(uint16_t*)b - *(uint16_t*)a;
}

uint16_t avg(uint16_t *buf, uint8_t size)
{
   uint8_t index;
   uint16_t sum = 0;
   
   for(index=0; index<size; index++)
   {
     sum += buf[index];
   }
   return sum/size;
}

BaseType_t tpad_driver_init(void)
{
    uint16_t buf[10];
    BaseType_t result;
    
    result = tpad_hardware_init();

    if(result == pdPASS)
    {
        /*read the capture value for no push, sort, used middle*/
        for(int i=0; i<TPAD_INIT_CHECK_TIMES;i++)
        {				 
            buf[i] = tpad_get_value();
            delay_ms(5);	    
        }
        qsort(buf, TPAD_INIT_CHECK_TIMES, sizeof(uint16_t), comp);
        no_push_value_ = avg(buf, TPAD_INIT_CHECK_TIMES-2);
        PRINT_LOG(LOG_INFO, "tpad no_push_value_:%d", no_push_value_);    
    }
    else
    {
        PRINT_LOG(LOG_INFO, "tpad_driver hardware_init failed");
    }
    return result;
}

uint16_t tpad_get_no_push_val(void)
{
    return no_push_value_;
}

uint16_t tpad_current_val(void)
{
    return current_value_;
}

static BaseType_t tpad_hardware_init(void)
{
    TIM_IC_InitTypeDef TIM2_CH1Config;  

    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2_tpad.Instance = TIM2;                       
    htim2_tpad.Init.Prescaler = TPAD_TIMER_PRESCALER;       
    htim2_tpad.Init.CounterMode = TIM_COUNTERMODE_UP;    
    htim2_tpad.Init.Period = TPAD_ARR_MAX_VAL;                   
    htim2_tpad.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_IC_Init(&htim2_tpad);

    TIM2_CH1Config.ICPolarity=TIM_ICPOLARITY_RISING;   
    TIM2_CH1Config.ICSelection=TIM_ICSELECTION_DIRECTTI;
    TIM2_CH1Config.ICPrescaler=TIM_ICPSC_DIV1;         
    TIM2_CH1Config.ICFilter=0;                        
    HAL_TIM_IC_ConfigChannel(&htim2_tpad, &TIM2_CH1Config, TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim2_tpad, TIM_CHANNEL_1);    
    
    return pdPASS;
}

uint8_t tpad_scan_key(void)
{
    /*get the max value when read capture.*/
    current_value_ = tpad_get_max_value(); 

    //check wheather is valid key push value.
    if(TPAD_IS_VALID_PUSH_KEY(current_value_, no_push_value_))					 
        return 1;  

    return 0;
}	

static void tpad_reset(void)
{
    GPIO_InitTypeDef GPIO_Initure;
	
    GPIO_Initure.Pin = GPIO_PIN_5;            
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  
    GPIO_Initure.Pull = GPIO_PULLDOWN;        
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;     
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);	
	
    //delay to wait capture run to zero
    delay_ms(3);	

    __HAL_TIM_CLEAR_FLAG(&htim2_tpad, TIM_FLAG_CC1|TIM_FLAG_UPDATE);   
    __HAL_TIM_SET_COUNTER(&htim2_tpad, 0); 
    
    GPIO_Initure.Mode = GPIO_MODE_AF_PP;      
    GPIO_Initure.Pull = GPIO_NOPULL;         
    GPIO_Initure.Alternate = GPIO_AF1_TIM2;  
    HAL_GPIO_Init(GPIOA, &GPIO_Initure); 
}

static uint16_t tpad_get_value(void)
{
    /*reset the pin for next capture.*/
    tpad_reset();
    
    while(__HAL_TIM_GET_FLAG(&htim2_tpad, TIM_FLAG_CC1) == RESET) 
    {
        //not more than TPAD_TIMES_CAPTURE_HIGH
        if(__HAL_TIM_GET_COUNTER(&htim2_tpad) > TPAD_TIMES_CAPTURE_HIGH) 
        {
            return __HAL_TIM_GET_COUNTER(&htim2_tpad);
        }
    };
    return HAL_TIM_ReadCapturedValue(&htim2_tpad,TIM_CHANNEL_1);
}

static uint16_t tpad_get_max_value(void)
{
    uint16_t temp; 
    uint16_t res = 0; 
    uint8_t okcnt = 0;
    uint8_t times = TPAD_CAPTURE_LOOP_TIMES;

    while(times--)
    {
        //get the capture value, store the max in res.
        temp = tpad_get_value();
        if(temp > res)
        {
            res = temp;
        }

        //key capture need in the scope, so include means valid.
        if(TPAD_IS_VALID_CAPTURE(temp))
        {
            okcnt++;
        }
    }

    if(okcnt >= TPAD_CAPTURE_LOOP_TIMES*2/3)
        return res;
    else 
        return 0;
}
  