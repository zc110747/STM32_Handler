
#include "dfu_test.h"
#include "arm_math.h"
 
void dfu_test(void)
{ 
    //float sqrt
    {
        float32_t in, out;
        q31_t inq, outq;
        
        in = 3.15;   
        if(arm_sqrt_f32(in, &out) == ARM_MATH_SUCCESS)
        {
            printf("dfu sqrt:%f, %f\r\n", in, out);
        }
    }   
    
    //float cos, sin
    {
        float32_t angle, outSin, outCos;
        
        angle = PI/4;
        outSin = arm_sin_f32(angle);
        outCos = arm_cos_f32(angle);
        printf("dfu sin:%f, cons:%f\r\n", outSin, outCos);
    }
}