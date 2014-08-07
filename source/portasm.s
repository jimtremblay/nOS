/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if defined(__GNUC__) && defined(__AVR__)
 #error "This file is not required. Do not build it."
#elif defined(__GNUC__) && defined(__ARM_ARCH_6M__)
 #error "This file is not required. Do not build it."
#elif defined(__GNUC__) && defined(__ARM_ARCH_7M__)
 #error "This file is not required. Do not build it."
#elif defined(__GNUC__) && defined(__ARM_ARCH_7EM__)
 #error "This file is not required. Do not build it."
#elif defined(__IAR_SYSTEMS_ASM__) && (__CORE__ == __ARM6M__)
 #include "port/IAR_Cortex_M0/iar_cortex_m0_asm.s"
#elif defined(__IAR_SYSTEMS_ASM__) && (__CORE__ == __ARM7M__)
 #include "port/IAR_Cortex_M3/iar_cortex_m3_asm.s"
#elif defined(__IAR_SYSTEMS_ASM__) && (__CORE__ == __ARM7EM__)
 #include "port/IAR_Cortex_M4F/iar_cortex_m4f_asm.s"
#elif defined(__CC_ARM) && defined(__TARGET_CPU_CORTEX_M3)
 #error "This file is not required. Do not build it."
#elif defined(__CC_ARM) && (defined(__TARGET_CPU_CORTEX_M4) || defined(__TARGET_CPU_CORTEX_M4_FP))
 #error "This file is not required. Do not build it."
#else
 #error "This compiler/processor is not yet supported."
#endif

    END
