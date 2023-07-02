```c
/*
1. LED输出	
硬件:GPIO-Output, PB0
状态:Complete

2. 按键key输入
硬件:GPIO-INPUT, PH2, PH3, PC13
状态:Complete

3. 外部中断
硬件:GPIO-INPUT, interrupt, PB12， 
状态:Complete

4. 独立看门狗
硬件:IWDG 
状态:Complete

5.串口通讯
硬件:USART TX-

6.I2C I/O扩展
硬件：I2C+PCF8574 SCL-PH4, SDA-PH5
状态:Complete

7.定时器中断,系统定时器
硬件:Timer，systick
状态:Complete

8.硬件随机数
硬件:RNG
状态:Complete

9.rtc实时时钟
硬件:RTC
状态:Complete


串口通讯	UART RX/TX, NVIC	Complete

PWM输出	Timer PWM	To do
输入捕获	Timer Compare	Complete
电容触摸按键	Timer Compare+Cap	Complete
TFTLCD(MCU屏)	FMC	Complete
SDRAM	FMC	Complete
RTC实时时钟	RTC	Complete
硬件随机数	RNG	Complete
ADC	ADC	Complete
内部温度传感器	ADC+Convert	Complete
DAC	DAC	In Process
DMA	DMA	To do

SPI	SPI+W25Q256	To do
CAN通讯	CAN	To do
触摸屏	I2C	To do
FLASH模拟EPPROM	FLASH	To do
SD卡	SDIO	In Process
Nand FLASH		To do
FATFS	FATFS	To do
汉字显示	字库	To do
		
FPU测试	fpu	To do
DSP测试	dsp	To do
串口IAP实验	IAP	To do
FreeRTOS相关	rtos	Complete
*/
```
