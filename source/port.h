/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PORT_H_
#define PORT_H_

#if defined(__GNUC__)
 #if defined(__AVR__)
  #include "port/GCC_AVR/gcc_avr.h"
 #else
  #error "This processor is not yet supported."
 #endif
#elif defined(__ICCARM__)
 #if __CORE__ == __ARM7EM__
  #include "port/IAR_Cortex_M4F/iar_cortex_m4f.h"
 #else
  #error "This processor is not yet supported."
 #endif
#else
 #error "This compiler is not yet supported."
#endif

#endif /* CPU_H_ */