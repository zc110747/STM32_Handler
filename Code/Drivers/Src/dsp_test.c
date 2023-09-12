
#include "dsp_test.h"

 
void dsp_app(void)
{ 
    //float sqrt
    {
        float32_t in, out;
        q31_t inq, outq;
        
        in = 3.15;   
        if(arm_sqrt_f32(in, &out) == ARM_MATH_SUCCESS)
        {
            PRINT_LOG(LOG_INFO, "dfu sqrt:%f, %f", in, out);
        }
    }   
    
    //float cos, sin
    {
        float32_t angle, outSin, outCos;
        
        angle = PI/4;
        outSin = arm_sin_f32(angle);
        outCos = arm_cos_f32(angle);
        PRINT_LOG(LOG_INFO, "dfu sin:%f, cons:%f", outSin, outCos);
    }
}