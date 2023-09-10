
_Pragma("once")


#include "key.h"
#include "usart.h"
#include "adc.h"
#include "i2c.h"
#include "spi.h"
#include "dma.h"
#include "dsp_test.h"

#include "drv_rtc.h"
#include "drv_led.h"
#include "drv_pwm.h"
#include "drv_tpad.h"
#include "drv_dac.h"
#include "drv_alg.h"
#include "drv_wdg.h"

#ifdef __cplusplus
	extern "C" {
#endif

BaseType_t driver_init();		
HAL_StatusTypeDef read_disk(uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks);
HAL_StatusTypeDef write_disk(const uint8_t *buf, uint32_t startBlocks, uint32_t NumberOfBlocks);
        
#ifdef __cplusplus
	}
#endif
