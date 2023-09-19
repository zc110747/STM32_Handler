/*
 * This file is part of the CmBacktrace Library.
 *
 * Copyright (c) 2020 Armink <armink.ztl@gmail.com>
 *                     Chenxuan <chenxuan.zhao@icloud.com>
 *
 * Permission is hereby granted free of charge to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software') to deal in the Software without restriction including
 * without limitation the rights to use copy modify merge publish
 * distribute sublicense and/or sell copies of the Software and to
 * permit persons to whom the Software is furnished to do so subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS' WITHOUT WARRANTY OF ANY KIND
 * EXPRESS OR IMPLIED INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM DAMAGES OR OTHER LIABILITY WHETHER IN AN ACTION OF CONTRACT
 * TORT OR OTHERWISE ARISING FROM OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * NOTE: DO NOT include this file on the header file.
 * Encoding: UTF-8
 * Created on: 2020-09-06
 */

#define PRINT_MAIN_STACK_CFG_ERROR   "ERROR: Unable to get the main stack information please check the configuration of the main stack"
#define PRINT_FIRMWARE_INFO          "Firmware name: %s hardware version: %s software version: %s"
#define PRINT_ASSERT_ON_THREAD       "Assert on thread %s"
#define PRINT_ASSERT_ON_HANDLER      "Assert on interrupt or bare metal(no OS) environment"
#define PRINT_THREAD_STACK_INFO      " Thread stack information "
#define PRINT_MAIN_STACK_INFO        " Main stack information "
#define PRINT_THREAD_STACK_OVERFLOW  "Error: Thread stack(%08x) was overflow"
#define PRINT_MAIN_STACK_OVERFLOW    "Error: Main stack(%08x) was overflow"
#define PRINT_CALL_STACK_INFO        "Show more call stack info by run: addr2line -e %s%s -a -f %s"
#define PRINT_CALL_STACK_ERR         "Dump call stack has an error"
#define PRINT_FAULT_ON_THREAD        "Fault on thread %s"
#define PRINT_FAULT_ON_HANDLER       "Fault on interrupt or bare metal(no OS) environment"
#define PRINT_REGS_TITLE             " Registers information "
#define PRINT_HFSR_VECTBL            "Hard fault is caused by failed vector fetch"
#define PRINT_MFSR_IACCVIOL          "Memory management fault is caused by instruction access violation"
#define PRINT_MFSR_DACCVIOL          "Memory management fault is caused by data access violation"
#define PRINT_MFSR_MUNSTKERR         "Memory management fault is caused by unstacking error"
#define PRINT_MFSR_MSTKERR           "Memory management fault is caused by stacking error"
#define PRINT_MFSR_MLSPERR           "Memory management fault is caused by floating-point lazy state preservation"
#define PRINT_BFSR_IBUSERR           "Bus fault is caused by instruction access violation"
#define PRINT_BFSR_PRECISERR         "Bus fault is caused by precise data access violation"
#define PRINT_BFSR_IMPREISERR        "Bus fault is caused by imprecise data access violation"
#define PRINT_BFSR_UNSTKERR          "Bus fault is caused by unstacking error"
#define PRINT_BFSR_STKERR            "Bus fault is caused by stacking error"
#define PRINT_BFSR_LSPERR            "Bus fault is caused by floating-point lazy state preservation"
#define PRINT_UFSR_UNDEFINSTR        "Usage fault is caused by attempts to execute an undefined instruction"
#define PRINT_UFSR_INVSTATE          "Usage fault is caused by attempts to switch to an invalid state (e.g. ARM)"
#define PRINT_UFSR_INVPC             "Usage fault is caused by attempts to do an exception with a bad value in the EXC_RETURN number"
#define PRINT_UFSR_NOCP              "Usage fault is caused by attempts to execute a coprocessor instruction"
#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M33)
    #define PRINT_UFSR_STKOF         "Usage fault is caused by indicates that a stack overflow (hardware check) has taken place"
#endif
#define PRINT_UFSR_UNALIGNED         "Usage fault is caused by indicates that an unaligned access fault has taken place"
#define PRINT_UFSR_DIVBYZERO0        "Usage fault is caused by Indicates a divide by zero has taken place (can be set only if DIV_0_TRP is set)"
#define PRINT_DFSR_HALTED            "Debug fault is caused by halt requested in NVIC"
#define PRINT_DFSR_BKPT              "Debug fault is caused by BKPT instruction executed"
#define PRINT_DFSR_DWTTRAP           "Debug fault is caused by DWT match occurred"
#define PRINT_DFSR_VCATCH            "Debug fault is caused by Vector fetch occurred"
#define PRINT_DFSR_EXTERNAL          "Debug fault is caused by EDBGRQ signal asserted"
#define PRINT_MMAR                   "The memory management fault occurred address is %08x"
#define PRINT_BFAR                   "The bus fault occurred address is %08x"


