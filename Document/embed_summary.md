# **嵌入式开发说明**
&emsp;&emsp;本篇将以STM32F429为例，讲解单片机中涉及的知识或者积累，对于一颗嵌入式芯片来说，由以下几部分组成.<br />
1. 内核Core和配套的调试系统(如目前常用的Cortex-M0, M3, M4, M7等)，这部分由ARM来设计。<br />
2. 外设模块，包含SPI，I2C, CAN，SDIO, USB等，这些模块由专业的IP厂商或者芯片厂商设计开发，并通过各类总线(AHB, APB等)与Core进行连接实现具体功能。<br />
3. 存储器，包含FLASH，RAM，其中FLASH部分支持I-Cache和D-Cache进行加速。<br />
4. 时钟，电源管理和复位系统，管理上电时序和提供系统和模块工作的电压和Clock，模块间的同步，寄存器的读取和修改都依赖时钟的触发，这部分由模拟模块构建，并依赖数字模块控制，是芯片正常工作的基础。<br />

## **1.内核概览**
&emsp;&emsp;芯片STM32F429IGT6基于Cortex-M4F内核设计，包含以下特点:<br />
1. 是32位的处理器内核，也就是说内部的寄存器和数据位宽都是32位的.<br />
2. 采用哈佛结构，具有独立的数据总线和指令总线.<br />
3. 支持MPU, 存储器保护单元, 可以控制对于不同memory的访问权限.<br />
4. 支持DSP指令集, 能够加速浮点运算.<br />
5. 支持handler和thread两种模式，分别用于表示异常服务例程和普通用户程序的代码，另外也支持特权分级，privileged和unprivileged模式, 其中handler模式只支持privileged模式.<br/>

### **1.1 通用寄存器**
&emsp;&emsp;Cortex-M4处理器拥有R0-R15寄存器组<br />

1. R0-R12为32位通用寄存器，用于数据操作。<br />
2. R13为堆栈指针寄存器，且同时指定两个堆栈指针:MSP(主堆栈指针)和PSP(进程堆栈指针)，并通过修改权限指向对应的堆栈指针，堆栈指针的最低两bit永远是0.<br />
3. R14为LR寄存器，主要在调用子程序时，存储返回地址.<br />
4. R15为PC寄存器，指向当前的程序地址.<br />
5. 特殊功能寄存器xPSR,用于记录ALU标志，管理中断以及修改系统的特权状态.<br />

&emsp;&emsp;这些寄存器的相关知识，主要在实现RTOS或者在调试hardfault时追踪，对于RTOS中，汇编指令如下<br />
```armasm
    msr psp, r0	 #将psp指针值写入r0寄存器，并将当前堆栈切换到PSP
    msr msp, r0	 #将msp指针值写入r0寄存器，并将当前堆栈切换到MSP
```
&emsp;&emsp;对于触发hardfault时，可以使用错误追踪库，则包含对上述寄存器的运用:https://github.com/armink/CmBacktrace<br />


### **1.2 中断向量控制器NVIC**
&emsp;&emsp;Cortex-M4内核支持NVIC中断向量控制器，其中最大支持240个可编程中断(由芯片厂商定义)，另外包含15个系统中断，这部分声明在startup_xxx.s启动文件中，例如stm32f4的定义如下:<br />
```s
    DCD     __initial_sp               ; Top of Stack
    DCD     Reset_Handler              ; Reset Handler          #复位中断
    DCD     NMI_Handler                ; NMI Handler            #不可屏蔽中断
    DCD     HardFault_Handler          ; Hard Fault Handler     #硬fault中断
    DCD     MemManage_Handler          ; MPU Fault Handler      #MPU访问异常
    DCD     BusFault_Handler           ; Bus Fault Handler      #总线错误异常
    DCD     UsageFault_Handler         ; Usage Fault Handler    #程序错误异常
    DCD     0                          ; Reserved
    DCD     0                          ; Reserved
    DCD     0                          ; Reserved
    DCD     0                          ; Reserved
    DCD     SVC_Handler                ; SVCall Handler         #系统SVC异常
    DCD     DebugMon_Handler           ; Debug Monitor Handler  #调试监视器异常
    DCD     0                          ; Reserved
    DCD     PendSV_Handler             ; PendSV Handler         #为系统设置的PendSVSS异常
    DCD     SysTick_Handler            ; SysTick Handler        #系统滴答定时器异常

    ; External Interrupts
    DCD     WWDG_IRQHandler            ; Window WatchDog
```
&emsp;&emsp;对于剩余240个中断，则由芯片厂商进行定义，当然也不是全部都存在，厂商根据外设的需求，可以自定义中断对应的线号, 如STM32F4来说，如头文件中WWDG_IRQn值为0，在中断向量表中就对应第一个外部中断WWDG_IRQHandler，在中断触发时，就通过查找在中断向量表中的偏移值，找到对应的中断函数，触发执行。<br />
&emsp;&emsp;另外对于Cortex-M4来说支持最大8位优先级，不过具体几位由芯片厂商定义，但是最少支持3bit，对于STM32F4则支持4bit作为优先级，且可以分为两段，分别为抢占优先级和子优先级，其中抢占优先级更高的中断，可以打断低优先级的中断， 对于中断处理的函数由内核提供，不过HAL库进行了封装，具体函数接口如下.<br />
```c
/*
设置中断向量的抢占优先级和子优先级
  *         @arg NVIC_PRIORITYGROUP_0: 0 bits for preemption priority
  *                                    4 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_1: 1 bits for preemption priority
  *                                    3 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_2: 2 bits for preemption priority
  *                                    2 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_3: 3 bits for preemption priority
  *                                    1 bits for subpriority
  *         @arg NVIC_PRIORITYGROUP_4: 4 bits for preemption priority
  *                                    0 bits for subpriority
*/
void HAL_NVIC_SetPriorityGrouping(uint32_t PriorityGroup)

//设置中断的抢占优先级和子优先级，0最高，最低优先级位2^bit
void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority)

//使能中断
void HAL_NVIC_EnableIRQ(IRQn_Type IRQn)

//关闭中断
void HAL_NVIC_DisableIRQ(IRQn_Type IRQn)

//设置中断向量表位置
SCB->VTOR = SRAM_BASE | ADDRESS; 
```
&emsp;&emsp;对于一个模块需要开启中断，在模块使能正常工作后，以usart为例，需要进行如下操作.<br />
```c
//开启模块内部中断使能
__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);

//设置中断对应的优先级
HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);

//使能中断
HAL_NVIC_EnableIRQ(USART1_IRQn);

//使能总中断
void __enable_irq(void);

//关闭总中断
void __disable_irq(void)

//定义中断的执行函数
void USART1_IRQHandler(void)
{
    uint8_t rx_data;
    
    if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
    {
        //对于接收中断标志位，当读取DR值时自动清除，因此不需要软件再清除
        if(HAL_UART_Receive(&huart1, &rx_data, 1, 100) == HAL_OK)
        {
            //update data to application
        }
    }
}
```
&emsp;&emsp;如果中断未正常触发，就可以查看上述步骤是否有缺失。这里描述下检查的方法.<br />

1. 查看非中断模式下模块能否正常收发数据，确定模块是否正常配置.<br />
2. 确定模块内部中断使能位是否置位。<br />
3. 确定NVIC中断使能位是否置位。<br />
4. 确定中断入口函数是否和启动文件内命名一致，如果没有对应中断函数，会停止在启动文件中。<br />
5. 如果上述4步骤仍然不触发，则判断是否关闭了总中断或者进行了中断屏蔽。<br />

&emsp;&emsp;上述就是分析中断无法执行的主要方式，当中断未正常触发时，一般按照此流程分析中断的触发流程。

### **1.3 调试接口**
&emsp;&emsp;Cortex-M4F内核提供集成再在片上调试支持，包含以下接口。
```
● SWJ-DP：串行/JTAG 调试端口
● AHP-AP：AHB 访问端口
● ITM：指令跟踪单元
● FPB：Flash 指令断点
● DWT：数据断点触发
● TPUI：跟踪端口单元接口（大封装上提供，其中会映射相应引脚）
● ETM：嵌入式跟踪宏单元（大封装上提供，其中会映射相应引脚）
```
&emsp;&emsp;基于调试工具，支持JTAG-DP(5脚)和SW-DP(2脚)模式，ETM嵌入式追踪单元使用TDI和TDO脚，与JTAG-DP模式下的JTDO和JTDI下一致。所以不能使用ETM作为打印调试。<br />

#### **segger-jlink调试**
&emsp;&emsp;对于调试打印，对于jlink，可以使用segger驱动内提供的SEGGER_RTT进行移植，将内部的**SEGGER_RTT.c**, **SEGGER_RTT_ASM_ARMv7M.S**和**SEGGER_RTT_printf.c**添加到代码中，添加头文件路径，即可使用**SEGGER_RTT.h**即可使用调试接口。<br />
```c
//安装地址
..\SEGGER\JLink\Samples\RTT

//BufferIndex - 写入port，目前为0
//pBuffer - 数据的起始指针
//NumBytes - 数据长度
unsigned SEGGER_RTT_Write (unsigned BufferIndex, const void* pBuffer, unsigned NumBytes);

//类似printf的可变输入函数
int SEGGER_RTT_printf(unsigned BufferIndex, const char * sFormat, ...);
```
&emsp;&emsp;使用工具J-Link RTT Client或J-Link RTT Viewer即可进行调试操作。<br />
#### **ST-Link调试**
&emsp;&emsp;对于ST-Link, 则可以使用ETM嵌入式跟踪宏单元进行调试，不过要确定以下设计.<br />
1. 使用SW-DP连接方式<br />
2. 确定TDI和TDO与芯片连接(比较简单的测试方法，支持JTAG模式连接).<br />

&emsp;&emsp;然后在代码中即可使用ETM接口进行跟踪调试。

```c
//需要在.c文件中声明ITM数据格式
volatile int32_t ITM_RxBuffer = ITM_RXBUFFER_EMPTY;

//ITM检查是否有数据读取
int32_t ITM_CheckChar (void);

//ITM读取数据
int32_t ITM_ReceiveChar (void);

//ITM发送数据
uint32_t ITM_SendChar (uint32_t ch);
```
&emsp;&emsp;之后可以使用调试接口下View->Serial Windows->Debug(printf) viewer进行输出，如果需要再非调试下查看，也可以使用STM32CubeProgrammer->SWV进行查看<br />
&emsp;&emsp;上面从内核，NVIC和调试部分应用讲解内核信息，不过这也仅仅只是内核功能中常用的一部分，如果想继续深入了解，可以参考以下书籍, 配合深入学习.<br />
```
<Cortex-M3权威指南>  - 宋岩译
<Cortex™-M4 Devices Generic User Guide> - ARM著
<STM32F4x 参考手册> - ST著
```

## **2.RCC时钟**
&emsp;&emsp;RCC是驱动系统运行的基础时钟模块，决定了系统内指令的执行时间，同时也提供系统欸各模块运行的基础时钟，对于模块的寄存器的访问，功能的实现都依赖时钟的工作。对于RCC模块，主要提供以下功能. <br />

1. 管理系统工作的时钟和外设模块时钟频率，让所有模块能够正常工作在相应的时钟频率上。<br />
2. 外设模块的时钟使能，关闭和复位的操作，这在模块使能和低功耗模式需要配合使用。<br />

### **2.1 时钟源和使能**
&emsp;&emsp;对于STM32F4来说，支持内部时钟源LSI(低速内部RC)和HSI(高速内部RC)，其中LSI为32kHz的内部RC振荡器，提供给看门狗工作时钟。高速内部RC则为16MHZ，可以直接提供给系统时钟或通过PLL电路提供给系统时钟。<br />
&emsp;&emsp;另外也支持外部时钟源LSE(低速外部时钟源)和HSE(高速外部时钟源)，其中LSE一般为32.768Khz,可用于代替内部LSI，HSE则为25MHz, 可用于代替内部HSI，具有相同的功能。外部时钟源主要可以使用两种晶振，分为有源晶振和无源晶振。<br />

1. 无源晶振是两个引脚的无极性器件，需要借助时钟电路(一般为电容配合内部激励时钟)才能产生震荡信号
2. 有源晶振是完整的振荡器，当正确接入电压后，会有引脚输出产生震荡信号

&emsp;&emsp;内部时钟源具有不依赖外部晶振器件，调试简单等优点，但是会因为设计有误差、温度的影响，相对来说不精确，对于高精度定时，高波特率串口通讯的场景会引入时钟误差，所以有这些功能需求的产品中一般使用外部晶振作为基础时钟源。对于嵌入式RCC开发，主要包含时钟使能，倍频和分频使能，然后将系统时钟，外设模块时钟选中使能的时钟源，下面为HAL库提供的时钟启动和配置的代码。<br />
```c
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** 
    * 开启系统的时钟源，并配置相应值
    * PLL依赖HAE, RTC工作采用LSE，IWDG采用LSI, 所以这些时钟需求开启
    * LSE Clock: 32.768KHz
    * LSI Clock: 32KHz
    * HSE Clock: 25MHz
    * System CLock: 25 / PLLM * PLLN / PLLP = 180MHz
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                          |RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 360;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Activate the Over-Drive mode
    */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    * SYSTICK Clock - PLL Clock 180MHz
    * AHB Clock - SYSTICK Clock/DIV 180MHz
    * APB1 Clock - AHB Clock/4 45MHz
    * APB2 Clock - AHB Clock/2 90MHz
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                            |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
```
### **2.1 外设模块时钟管理**
&emsp;&emsp;RCC模块提供其它外设的模块管理功能，通过RCC_APBxRSTR, RCC_AHBxRSTR, RCC_APBxENR, RCC_AHBxENR寄存器，可以控制大部分外设的模块使能，关闭和复位功能，这些在HAL库倍封装成宏进行操作, 下面举例来列出些接口说明。
```c
//使能GPIOA的时钟
__HAL_RCC_GPIOA_CLK_ENABLE();

//关闭GPIOA的时钟
__HAL_RCC_GPIOA_CLK_DISABLE();

//复位GPIOA模块
__HAL_RCC_GPIOA_FORCE_RESET();
```
&emsp;&emsp;上述就是RCC提供的用于模块时钟管理接口，对于后续使用的外设，如SPI，I2C， Timer等，都需要进行相应的时钟使能。

## **3. GPIO模块**
&emsp;&emsp;GPIO是芯片与外部连接的输入输出口(IO)，外部的按键输入，点亮控制LED灯，蜂鸣器，连接I2C, SPI, USART和CAN设备，在最底层都依赖于GPIO进行实际信号的交互，对于简单的输入，输出，使用GPIO模块就可以完成输入输出动作，对于复杂的协议应用，需要在GPIO复用到对应的功能后，在使能配置相应的模块，通过更高级的外设模块信号来检测或者控制GPIO的输入输出，从而实现具体的协议电平输出，这也是SPI和I2C这些协议能够通过软件操作GPIO来进行模拟的原因，下面用STM32F4中GPIO设计接口展示更详细的说明。<br />
![image](image/1_IO.JPG#pic_center)<br />
&emsp;&emsp;从上图可以看出，GPIO模块当打开输入功能时，接收到的数据会写入输入数据寄存器，如果使用复用功能，则同时也会通过复用功能输入信号到达片上外设，打开输出功能时，则可通过复位/置位寄存器写入输出数据寄存器(也可直接写入),另外复用功能输出会直接输出到外部I/O中，这与上面说明的功能一致，另外输出部分可以通过控制Vdd的MOS开关，表示I/O为开漏或者推挽模式，另外在GPIO对应的引脚中也包含可开关的上拉和下拉电阻，分别控制I/O初始化的默认输出值，这在输入模式下可以避免启动时的误触发从这张图在结合上章RCC的内容，则将GPIO配置为输出的初始化和控制的代码如下所示。<br />
```c
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    //使能GPIOB对应的RCC时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();

    //写入GPIO的默认值，避免初始化后误输出
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;     //推挽输出，P, N MOS都支持控制
    GPIO_InitStruct.Pull = GPIO_NOPULL;             //无外部上拉/下拉电阻，关闭PULL
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;    //控制I/O的输出速率，有协议或者输出速率的要求时改为更高。

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
```
&emsp;&emsp;配置为输入则类似，具体代码如下。<br />
```c
    GPIO_InitTypeDef GPIO_InitStruct = {0};

     //使能GPIOC对应的RCC时钟
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;         //输入模式
    GPIO_InitStruct.Pull = GPIO_PULLUP;             //有外部上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;   
    
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
```
&emsp;&emsp;其中Pull和Speed分别对应外部上拉/下拉电阻的控制，Speed则对应I/O允许的最大速率，Mode则比较重要，用来配置I/O的具体功能，具体如下.<br />
```c
    GPIO_MODE_INPUT                 //输入模式
    GPIO_MODE_OUTPUT_PP             //推挽输出
    GPIO_MODE_OUTPUT_OD             //开漏模式，需要外部上拉才能输出高
    GPIO_MODE_AF_PP                 //复用为其它外设控制，同时GPIO推挽
    GPIO_MODE_AF_OD                 //复用为其它外设控制，同时GPIO开漏
    GPIO_MODE_ANALOG                //模拟模式，主要用于ADC和DAC应用
    //中断相关配置参考外部中断说明
    //......
```
&emsp;&emsp;对于复用模式，除了将模式配置为复用外，还需要将I/O和对应的外设关联起来，在数据手册上会提供映射的表格，如果使用STM32CubeMX将更为简单，将I/O配置成对应的外设模块对应功能，则代码中会生成对应的复用接口，如下所示。<br />
```c
    __HAL_RCC_I2C2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    /**I2C2 GPIO Configuration
    PH4     ------> I2C2_SCL
    PH5     ------> I2C2_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;      //PH4， PH5复用到I2C2， 使用AF4通道
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
```
&emsp;&emsp;对于GPIO的操作主要包含读和写，具体如下.<br />
```c
    //读取指定端口I/O电平
    HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

    //写入指定端口I/O电平
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
```
&emsp;&emsp;在本章讲述了GPIO的功能，包含初始化，读写，以及如何配合外设模块复用功能实现具体通讯的功能，可以看到通讯外设模块也是基于GPIO实现最终的功能，这除了在开发软件时注意外，也提供了调试硬件接口连通的方法，当两个芯片通过I2C，USART或者SPI连接不通时。<br />

1. 可以先将引脚配置为普通GPIO输入/输出模式，看双方是否能检测到正确的电平，这样就可以快速判断是否为硬件问题。<br />
2. 如果不能检测到，先去排查硬件问题，反之，则查看引脚代码中是否开启的复用功能，且配置是否与芯片的定义一致，一致则再去查上层的模块配置问题，不一致则修改后再调试。<br />

&emsp;&emsp;这里全面讲解了GPIO的功能，下一章则在此基础上讲述外部中断。<br />

## **4.外部中断**
&emsp;&emsp;外部中断/事件是在GPIO基础上，检测按键状态变化的功能。在配置了GPIO为输入后，在开启配置相应的EXTI模块和NVIC中对应的中断，就可以实现当状态变化时，触发对应的中断和事件，执行对应的中断函数或者置位相应的事件，用于软件的进一步处理。<br />
![image](image/2_EXTI_EVENT.JPG#pic_center)<br />
&emsp;&emsp;结合上图，可以看出GPIO的输入信号通过边沿检测电路(可独立控制开启上升沿或者下降沿检测)，输出可通过软件中断事件寄存器控制，决定是否进一步转换成中断事件，并通过中断或者事件寄存器来控制具体触发中断还是通过控制脉冲发生器，产生event信号，用于软件具体处理。另外也可以看到边沿检测基于APB总线的时钟，所以只有频率低于APB2时钟的外部信号才能被正确检测，否则就可能会有遗漏，关于外部中断在代码中应用实现如下。
```c
    //使能GPIOB的时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin : PB12 */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; //配置下降沿触发，软件中断事件寄存器
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);         //配置对应的中断屏蔽寄存器，打开中断
```
&emsp;&emsp;可以看到，因为HAL库的集成，Mode即可配置上升/下降沿产生中断，另外为了避免开机误触中断，符合外部检测的要求，也要相应的配置GPIO的外部上拉电阻保证默认电平为高，关于中断和模式的配置选项如下所示.
```c
    GPIO_MODE_IT_RISING             //输入模式，上升沿触发中断
    GPIO_MODE_IT_FALLING            //输入模式，下降沿触发中断
    GPIO_MODE_IT_RISING_FALLING     //输入模式，上升沿，下降沿都触发中断
    GPIO_MODE_EVT_RISING            //输入模式，上升沿触发事件
    GPIO_MODE_EVT_FALLING           //输入模式，上升沿触发事件
    GPIO_MODE_EVT_RISING_FALLING    //输入模式，上升沿，下降沿都触发事件
```
&emsp;&emsp;对于中断的实际执行，当有外部符合的信号触发时，则执行对应的中断函数，对于EXTI15_10_IRQn来说，执行的中断就是EXTI15_10_IRQHandler, 对于共用的中断信号，读取对应的中断的信号，采用如下即可检测和执行中断。
```c
void EXTI15_10_IRQHandler(void)
{
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET)
    {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
        
        //执行具体的中断触发,中断中不建议具体的delay，可以触发后，在其它地方进行周期读取检测
    }
}
```
&emsp;&emsp;对于事件，可以作为其它模块的触发条件，如ADC支持外部EXTI线-11进行触发。中断可以通过软件调用函数实现各种功能，事件则提供信号用于作为其它外设的触发条件，例如低功耗模式和事件唤醒就是stm32支持的事件之一。

## **5.RTC时钟管理**
&emsp;&emsp;实时时钟(RTC)是一个独立的BTC定时器/计数器。RTC提供日历时钟，可编程的闹钟中断，以及具有中断功能的周期性可编程唤醒标志。RTC中包含秒，分钟，小时，星期，日期，月份和年份，且可以自动补偿月份和天数。RTC支持使用外部时钟LSE，HSE的一定分频数或者LSI作为时钟的输入。对于RTC的工作，需要提供最终的1Hz的时钟，由一个7位的异步预分频器(fdiv_a)和一个15位的同步预分频器(fdiv_s)分频获得最后的时钟，为了降低功耗，建议异步分频器的值选择尽可能大, 转换公式如下所示。<br />
```c
fclk = (fbase)/((fdiv_a + 1) * (fdiv_s + 1);
```
&emsp;&emsp;当选用LSE(时钟为32768Hz)时，拆分成128*256, 则fdiv_a值为127，fdiv_s值为255.对于RTC的寄存器，上电复位后，所有RTC寄存器均受到写保护，需要通过写入PWR_CR的DBP位才能使能RTC寄存器的写访问, RTC时钟包含最基础的掉电计时，闹钟以及唤醒功能，可以有独立的电池供电，从而实现在系统掉电后能够正常计时，关于RTC的供电设计一般如下所示.<br />
![image](image/3_RTC.JPG#pic_center)<br />
&emsp;&emsp;一般选用外部3.3V和电池通过BAT54c二极管进行切换供电，这是因为使用的纽扣电池电压一般为3V,当系统有供电时，电压高于电池电压，所有使用外部供电，当系统断电时，此时电池端有电，电压就切换到3V，RTC模块仍然能保证正常工作，这种模式可以避免带电工作时仍然使用电池供电，延长工作时间。RTC时钟支持通过I/O输出，用于校准，校准配置到对应的校准寄存器内，这一般在高精度定时器计时时需要，不过对于此场景，如果能够连接外网，可以使用NTP服务器进行时钟校准，如果不能连接，可以直接使用外部RTC模块实现更高的定时，也更简单易操作，且能保证可靠性。RTC同时支持一组20个4字节的备份寄存器，上电和复位数据不丢失，我们可以用这个寄存器来确认RTC是否被配置过，从而避免重复修改初始时间，导致时间恢复默认值从而不准确。<br />
&emsp;&emsp;结合上面说明，RTC的配置如下所示，至于读取时钟，则需要先读取时间，再读取日期，这是因为设计上为了确保这 3个值来自同 一时刻点，读取 RTC_SSR 或 RTC_TR 时会锁定高阶日历影子寄存器中的值，直到读取RTC_DR，也就是SSR和TR读取后，DR才更新。<br />
```c
    //允许RTC寄存器修改
    HAL_PWR_EnableBkUpAccess();

    rtc_handler_.Instance = RTC;
    rtc_handler_.Init.HourFormat = RTC_HOURFORMAT_24;   //设置计时模式
    rtc_handler_.Init.AsynchPrediv = 127;               //设置异步和同步分频，保证时钟为1Hz.
    rtc_handler_.Init.SynchPrediv = 255;
    rtc_handler_.Init.OutPut = RTC_OUTPUT_DISABLE;      //不需要输出内部时钟用于校准，否则使用RTC_AF1进行输出
    rtc_handler_.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    rtc_handler_.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&rtc_handler_) != HAL_OK)
        return pdFAIL;

    if(HAL_RTCEx_BKUPRead(&rtc_handler_, RTC_BKP_DR0) != RTC_SET_FLAGS)
    {
        //写入当前的时:分:秒
        time_.Hours = hour;
        time_.Minutes = min;
        time_.Seconds = sec;
        time_.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        time_.StoreOperation = RTC_STOREOPERATION_RESET;
        HAL_RTC_SetTime(&rtc_handler_, &time_, RTC_FORMAT_MODE);

        //写入当前的年:月:日
        date_.Year = year;
        date_.Month = month;
        date_.Date = date;
        date_.WeekDay = week;
        HAL_RTC_SetDate(&rtc_handler_, &date_, RTC_FORMAT_MODE);

        HAL_RTCEx_BKUPWrite(&rtc_handler_, RTC_BKP_DR0, RTC_SET_FLAGS);
    }

    //获取RTC时钟
    HAL_RTC_GetTime(&rtc_handler_, &time_, RTC_FORMAT_MODE);
    
    //获取RTC日期
    HAL_RTC_GetDate(&rtc_handler_, &date_, RTC_FORMAT_MODE);
```
&emsp;&emsp;对于RTC内部，寄存器使用BCD格式来保存上述信息，这里面讲解下BCD码和二进制的转换，二进制数字12转换成BCD就是(1<<4 | 2),高位为12/10，低位为12%10， 下面展示时间的转换.<br />
```C
  RTC_FORMAT_BIN模式对应时间 17:23:00
  RTC_FORMAT_BCD模式对应 
  时 (17/10)<<4 | (17%10) = 0x17
  分 (23/10)<<4 | (23%10) = 0x23
  秒 (00/10)<<4 | (00%10) = 0x00
```
&emsp;&emsp;另外RTC也支持闹钟功能，且可以用于唤醒休眠的内核，关于闹钟功能，除支持时分秒和星期的配置，另外也可以配置不比较，下面配置选择所有位有效，配置的代码如下.<br />
```c
    //设置定时的时分秒
    rtc_arm_handler_.AlarmTime.Hours= hour;  
    rtc_arm_handler_.AlarmTime.Minutes = min; 
    rtc_arm_handler_.AlarmTime.Seconds = sec; 
    rtc_arm_handler_.AlarmTime.SubSeconds = 0;
    rtc_arm_handler_.AlarmTime.TimeFormat = RTC_HOURFORMAT_24;
    
    //设置不参与比较位和星期，选择对应的RTC闹钟，支持A和B两种
    rtc_arm_handler_.AlarmMask = RTC_ALARMMASK_NONE;
    rtc_arm_handler_.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
    rtc_arm_handler_.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
    rtc_arm_handler_.AlarmDateWeekDay = week; 
    rtc_arm_handler_.Alarm = RTC_ALARM_A;     
    
    HAL_RTC_SetAlarm_IT(&rtc_handler_, &rtc_arm_handler_, RTC_FORMAT_MODE);
    
    //设置RTC优先级并使能
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0x01, 0x02);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
```
&emsp;&emsp;对于RTC时间到达后，触发的中断如下所示。
```c
    void RTC_Alarm_IRQHandler(void)
    {
        RTC_HandleTypeDef *prtc_handler = &rtc_handler_;

        //判断定时器A触发
        if(__HAL_RTC_ALARM_GET_IT(prtc_handler, RTC_IT_ALRA))
        {
            if((uint32_t)(prtc_handler->Instance->CR & RTC_IT_ALRA) != (uint32_t)RESET)
            {
                is_alarm = pdTRUE;
                    
                /* Clear the Alarm interrupt pending bit */
                __HAL_RTC_ALARM_CLEAR_FLAG(prtc_handler,RTC_FLAG_ALRAF);
            }
        }

        /* Clear the EXTI's line Flag for RTC Alarm */
        __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();

        /* Change RTC state */
        prtc_handler->State = HAL_RTC_STATE_READY; 
    }
```
&emsp;&emsp;如此便是RTC时钟和闹钟功能的大致说明，当然这不包含全部的功能，如RTC如何精确校准，如何使用RTC来唤醒休眠的芯片，这部分更深入的应用只有使用到才需要进一步去了解，这里就不在赘述。

## **6.ADC检测**
//....

## **7.DAC输出**

