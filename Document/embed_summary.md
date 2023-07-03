# **嵌入式开发说明**
本篇将以STM32F429IGT6为例，讲解单片机中涉及的知识或者积累，对于一颗嵌入式芯片来说，由以下几部分组成.<br />
```
1.内核Core和配套的调试系统(如目前常用的Cortex-M0, M3, M4, M7等)，这部分由ARM来设计。
2.外设模块，包含SPI，I2C, CAN，SDIO, USB等，这些模块由专业的IP厂商或者芯片厂商设计开发，并通过各类总线(AHB, APB等)与Core进行连接实现具体功能。
3.存储器，包含FLASH，RAM，其中FLASH部分支持I-Cache和D-Cache进行加速。
4.时钟，电源管理和复位系统，管理上电时序和提供系统和模块工作的电压和Clock，模块间的同步，寄存器的读取和修改都依赖时钟的触发，这部分由模拟模块构建，并依赖数字模块控制，是芯片正常工作的基础。
```
## **1.内核概览**
芯片STM32F429IGT6基于Cortex-M4F内核设计，包含以下特点:<br />
&emsp;&emsp;1.是32位的处理器内核，也就是说内部的寄存器和数据位宽都是32位的.<br />
&emsp;&emsp;2.采用哈佛结构，具有独立的数据总线和指令总线.<br />
&emsp;&emsp;3.支持MPU, 存储器保护单元, 可以控制对于不同memory的访问权限.<br />
&emsp;&emsp;4.支持DSP指令集, 能够加速浮点运算.<br />
&emsp;&emsp;5.支持handler和thread两种模式，分别用于表示异常服务例程和普通用户程序的代码，另外也支持特权分级，privileged和unprivileged模式, 其中handler模式只支持privileged模式.<br/>
### **1.1 通用寄存器**
Cortex-M4处理器拥有R0-R15寄存器组<br />
&emsp;&emsp;1.R0-R12为32位通用寄存器，用于数据操作。<br />
&emsp;&emsp;2.R13为堆栈指针寄存器，且同时指定两个堆栈指针:MSP(主堆栈指针)和PSP(进程堆栈指针)，并通过修改权限指向对应的堆栈指针，堆栈指针的最低两bit永远是0.<br />
&emsp;&emsp;3.R14为LR寄存器，主要在调用子程序时，存储返回地址.<br />
&emsp;&emsp;4.R15为PC寄存器，指向当前的程序地址.<br />
&emsp;&emsp;5.特殊功能寄存器xPSR,用于记录ALU标志，管理中断以及修改系统的特权状态.<br />
这些寄存器的相关知识，主要在实现RTOS或者在调试hardfault时追踪，对于RTOS中，汇编指令如下<br />
```s
    msr psp, r0	 #将psp指针值写入r0寄存器，并将当前堆栈切换到PSP
    msr msp, r0	 #将msp指针值写入r0寄存器，并将当前堆栈切换到MSP
```
对于触发hardfault时，可以使用错误追踪库，则包含对上述寄存器的运用:https://github.com/armink/CmBacktrace<br />

### **1.2 中断向量控制器NVIC**
Cortex-M4内核支持NVIC中断向量控制器，其中最大支持240个可编程中断(由芯片厂商定义)，另外包含15个系统中断，这部分声明在startup_xxx.s启动文件中，例如stm32f4的定义如下:<br />
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
对于剩余240个中断，则由芯片厂商进行定义，当然也不是全部都存在，厂商根据外设的需求，可以自定义中断对应的线号, 如STM32F4来说，如头文件中WWDG_IRQn值为0，在中断向量表中就对应第一个外部中断WWDG_IRQHandler，在中断触发时，就通过查找在中断向量表中的偏移值，找到对应的中断函数，触发执行。<br />
另外对于Cortex-M4来说支持最大8位优先级，不过具体几位由芯片厂商定义，但是最少支持3bit，对于STM32F4则支持4bit作为优先级，且可以分为两段，分别为抢占优先级和子优先级，其中抢占优先级更高的中断，可以打断低优先级的中断， 对于中断处理的函数由内核提供，不过HAL库进行了封装，具体函数接口如下.<br />
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
对于一个模块需要开启中断，在模块使能正常工作后，以usart为例，需要进行如下操作.<br />
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
如果中断未正常触发，就可以查看上述步骤是否有缺失。这里描述下检查的方法.<br />
&emsp;&emsp;1.查看非中断模式下模块能否正常收发数据，确定模块是否正常配置.<br />
&emsp;&emsp;2.确定模块内部中断使能位是否置位。<br />
&emsp;&emsp;3.确定NVIC中断使能位是否置位。<br />
&emsp;&emsp;4.确定中断入口函数是否和启动文件内命名一致，如果没有对应中断函数，会停止在启动文件中。<br />
&emsp;&emsp;5.如果上述4步骤仍然不触发，则判断是否关闭了总中断或者进行了中断屏蔽。<br />
上述就是分析中断无法执行的主要方式。

### **1.3 调试接口**
Cortex-M4F内核提供集成再在片上调试支持，包含以下接口。
```
● SWJ-DP：串行/JTAG 调试端口
● AHP-AP：AHB 访问端口
● ITM：指令跟踪单元
● FPB：Flash 指令断点
● DWT：数据断点触发
● TPUI：跟踪端口单元接口（大封装上提供，其中会映射相应引脚）
● ETM：嵌入式跟踪宏单元（大封装上提供，其中会映射相应引脚）
```
基于调试工具，支持JTAG-DP(5脚)和SW-DP(2脚)模式，ETM嵌入式追踪单元使用TDI和TDO脚，与JTAG-DP模式下的JTDO和JTDI下一致。所以不能使用ETM作为打印调试。<br />
#### **segger-jlink调试**
对于调试打印，对于jlink，可以使用segger驱动内提供的SEGGER_RTT进行移植，将内部的**SEGGER_RTT.c**, **SEGGER_RTT_ASM_ARMv7M.S**和**SEGGER_RTT_printf.c**添加到代码中，添加头文件路径，即可使用**SEGGER_RTT.h**即可使用调试接口。
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
使用工具J-Link RTT Client或J-Link RTT Viewer即可进行调试操作。<br />
#### **ST-Link调试**
对于ST-Link, 则可以使用ETM嵌入式跟踪宏单元进行调试，不过要确定以下设计.<br />
&emsp;&emsp;1.使用SW-DP连接方式<br />
&emsp;&emsp;2.确定TDI和TDO与芯片连接(比较简单的测试方法，支持JTAG模式连接).<br />
然后在代码中即可使用ETM接口进行跟踪调试。
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
之后可以使用调试接口下View->Serial Windows->Debug(printf) viewer进行输出，如果需要再非调试下查看，也可以使用STM32CubeProgrammer->SWV进行查看<br />
上面从内核，NVIC和调试部分应用讲解内核信息，不过这也仅仅只是内核功能中常用的一部分，如果想继续深入了解，可以参考以下书籍, 配合深入学习.<br />
```
<Cortex-M3权威指南>  - 宋岩译
<Cortex™-M4 Devices Generic User Guide> - ARM著
<STM32F4x 参考手册> - ST著
```

## **2.RCC时钟**