/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if defined(__IAR_SYSTEMS_ASM__)
 #if __CORE__ == __ARM7EM__
  #include "port/IAR_Cortex_M4F/iar_cortex_m4f_asm.s"
 #else
  #error "This processor is not yet supported."
 #endif
 END
#elif !defined(__GNUC__)
 #error "This compiler is not yet supported."
#endif

