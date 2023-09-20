_Pragma("once")

#include "interface.h"

#define SPI_USE_HARDWARE        0
#define SPI_USE_SOFTWARE        1

#define SPI_RUN_MODE            SPI_USE_SOFTWARE   

#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg1		0x05 
#define W25X_ReadStatusReg2		0x35 
#define W25X_ReadStatusReg3		0x15 
#define W25X_WriteStatusReg1    0x01 
#define W25X_WriteStatusReg2    0x31 
#define W25X_WriteStatusReg3    0x11 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 
#define W25X_Enable4ByteAddr    0xB7
#define W25X_Exit4ByteAddr      0xE9

#define SPI_CS_ON   HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
#define SPI_CS_OFF  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);

typedef enum
{
    w25_q8,
    w25_q16,
    w25_q32,
    w25_q64,
    w25_q128,
    w25_q256
}chip_type;

#define CHIP_ID_Q8      0xEF13
#define CHIP_ID_Q16     0xEF14
#define CHIP_ID_Q32     0xEF15
#define CHIP_ID_Q64     0xEF16
#define CHIP_ID_Q128    0xEF17
#define CHIP_ID_Q256    0xEF18

#ifdef __cplusplus
    extern "C" {
#endif

//spi interface
BaseType_t spi_driver_init(void);
uint8_t spi_rw_byte(uint8_t data);
        
//wq interface
BaseType_t spi_wq_driver_init(void);
uint16_t wq_read_id(void);
void wq_write_enable(void);
void wq_write_disable(void);
void wq_read(uint8_t *pbuffer, uint32_t addr, uint16_t num);
void wq_erase_sector(uint32_t sector);
uint16_t wq_write_page(uint8_t *pbuffer, uint32_t addr, uint16_t num);        
#ifdef __cplusplus
    }
#endif
