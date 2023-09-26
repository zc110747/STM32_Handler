//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2023-by Persional Inc.  
//  All Rights Reserved
//
//  Name:
//      dsp.c
//
//  Purpose:
//      the dsp interface support by arm lib library.
//      1. floating Point hardware use signal precision
//      2. include arm header "arm_const_structs.h" or "arm_math.h"
//      3. include dsp library arm_cortexM4lf_math.lib
// Author:
//      @zc
//
//  Assumptions:
//	
//  Revision History:
//
/////////////////////////////////////////////////////////////////////////////
#include "dsp.h"
#include "arm_const_structs.h"
 
BaseType_t dsp_app(void)
{ 
    //float sqrt
    {
        float32_t in, out;
        q31_t inq, outq;
        
        in = 3.15;   
        if(arm_sqrt_f32(in, &out) == ARM_MATH_SUCCESS)
        {
            PRINT_LOG(LOG_INFO, "dfu sqrt:%f, %f.", in, out);
        }
    }   
    
    //float cos, sin
    {
        float32_t angle, outSin, outCos;
        
        angle = PI/4;
        outSin = arm_sin_f32(angle);
        outCos = arm_cos_f32(angle);
        PRINT_LOG(LOG_INFO, "dfu sin:%f, cons:%f.", outSin, outCos);
    }
    

    {
        float32_t vector[] = {1.2, 2.5, 3.5, 11.2, 23.4, 12.5};
        float32_t sum_quare = 0.0;
        float32_t mean_val = 0.0;
        float32_t var_val = 0.0;
        float32_t rms_val = 0.0;
        float32_t std_val = 0.0;
        float32_t min_val = 0.0;
        uint32_t min_index = 0;
        float32_t max_val = 0.0;
        uint32_t max_index = 0;
        
        uint16_t size = sizeof(vector)/sizeof(float32_t);
        
        //sum of square
        arm_power_f32(vector, size, &sum_quare);
        
        //mean
        arm_mean_f32(vector, size, &mean_val);
        
        //variance
        arm_var_f32(vector, size, &var_val);
        
        //root mean square
        arm_rms_f32(vector, size, &rms_val);
        
        //standard deviation
        arm_std_f32(vector, size, &std_val);        
        
        //min
        arm_min_f32(vector, size, &min_val, &min_index);

        //max
        arm_max_f32(vector, size, &max_val, &max_index);
        
        PRINT_LOG(LOG_INFO, "quare:%f, mean:%f, variance:%f, rms:%f.", sum_quare, mean_val, var_val, rms_val);     
        PRINT_LOG(LOG_INFO, "std:%f, min:%d, %f, max:%d, %f", std_val, min_index, min_val, max_index, max_val);          
    }
    
    {
        //mult float vector
        float32_t vector1[] = {1.5, 2.5, 6.2};
        float32_t vector2[] = {2.1, 3.5, 5.4};
        float32_t vector3[] = {0.0, 0.0, 0.0};
        arm_mult_f32(vector1, vector2, vector3, 3);
        PRINT_LOG(LOG_INFO, "vector:%f, %f, %f", vector3[0], vector3[1], vector3[2]);   
    }
    
    //fft
    {
        #define FFT_SRC_SIZE 256
        static float32_t fft_buffer[FFT_SRC_SIZE*2];
        static float32_t fft_outbuffer[FFT_SRC_SIZE*2];
        uint16_t i;
        
        for(i=0; i<FFT_SRC_SIZE; i++)
        {
            //50Hz正弦波,起始相位60°
            fft_buffer[i*2] = 1+arm_cos_f32(2*PI*50*i/FFT_SRC_SIZE+PI/3);
            fft_buffer[i*2+1] = 0;
        }
        
        //cfft变换
        arm_cfft_f32(&arm_cfft_sR_f32_len256, fft_buffer, 0, 1);
        
        //求解模值
        arm_cmplx_mag_f32(fft_buffer, fft_outbuffer, FFT_SRC_SIZE);
    }
    
    return pdPASS;
}