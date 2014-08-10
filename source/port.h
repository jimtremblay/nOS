/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PORT_H
#define PORT_H

#if defined(__GNUC__) && defined(__AVR__)
 #include "port/GCC_AVR/gcc_avr.h"
#elif defined(__GNUC__) && defined(__ARM_ARCH_6M__)
 #include "port/GCC_Cortex_M0/gcc_cortex_m0.h"
#elif defined(__GNUC__) && defined(__ARM_ARCH_7M__)
 #include "port/GCC_Cortex_M3/gcc_cortex_m3.h"
#elif defined(__GNUC__) && defined(__ARM_ARCH_7EM__)
 #include "port/GCC_Cortex_M4F/gcc_cortex_m4f.h"
#elif defined(__ICCARM__) && (__CORE__ == __ARM6M__)
 #include "port/IAR_Cortex_M0/iar_cortex_m0.h"
#elif defined(__ICCARM__) && (__CORE__ == __ARM7M__)
 #include "port/IAR_Cortex_M3/iar_cortex_m3.h"
#elif defined(__ICCARM__) && (__CORE__ == __ARM7EM__)
 #include "port/IAR_Cortex_M4F/iar_cortex_m4f.h"
#elif defined(__CC_ARM) && (defined(__TARGET_CPU_CORTEX_M0) || defined(__TARGET_CPU_CORTEX_M0_))
 #include "port/MDK_Cortex_M0/mdk_cortex_m0.h"
#elif defined(__CC_ARM) && defined(__TARGET_CPU_CORTEX_M3)
 #include "port/MDK_Cortex_M3/mdk_cortex_m3.h"
#elif defined(__CC_ARM) && (defined(__TARGET_CPU_CORTEX_M4) || defined(__TARGET_CPU_CORTEX_M4_FP))
 #include "port/MDK_Cortex_M4F/mdk_cortex_m4f.h"
#else
 #error "This compiler/processor is not yet supported."
#endif

#endif /* PORT_H */
