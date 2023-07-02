
#include "dfu_test.h"
#include "arm_math.h"


#define	DELTA	0.00005f		//误差值
 
//sin cos测试
//angle:起始角度
//times:运算次数
//mode:0,不使用DSP库;1,使用DSP库
//返回值：0,成功;0XFF,出错
uint8_t sin_cos_test(float angle,uint32_t times,uint8_t mode)
{
	float sinx,cosx;
	float result;
	uint32_t i=0;
	if(mode==0)
	{
		for(i=0;i<times;i++)
		{
			cosx=cosf(angle);			//不使用DSP优化的sin，cos函数
			sinx=sinf(angle);
			result=sinx*sinx+cosx*cosx; //计算结果应该等于1  
			result=fabsf(result-1.0f);	//对比与1的差值
			if(result>DELTA)return 0XFF;//判断失败 
			angle+=0.001f;				//角度自增
		}
	}else
	{
		for(i=0;i<times;i++)
		{
            
			cosx=arm_cos_f32(angle);	//使用DSP优化的sin，cos函数
			sinx=arm_sin_f32(angle);
			result=sinx*sinx+cosx*cosx; //计算结果应该等于1  
			result=fabsf(result-1.0f);	//对比与1的差值
			if(result>DELTA)return 0XFF;//判断失败 
			angle+=0.001f;				//角度自增
		}
	}
	return 0;//任务完成
} 

void dfu_test(void)
{
    uint8_t a, b;
    a = sin_cos_test(PI/4, 10, 0);
    b = sin_cos_test(PI/4, 10, 1);
    printf("dfu test, a:0x%x, b:0x%x\r\n", a, b);
}