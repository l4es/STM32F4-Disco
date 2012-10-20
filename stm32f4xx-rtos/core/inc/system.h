/***************************************************************************
 * system.h
 * (C) 2010 Ivan Meleca
 * Based on original code written by Ruben Meleca
 * www.milos.it

#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __CORE_H__
#define __CORE_H__

#include <plat_cpu.h>


/** @addtogroup System
  * @brief System functions.
  * @{
  */

/** @defgroup System_FunctionPrototypes Prototypes
  * @{
  */

typedef	__VOID (__APP_ENTRY)(__VOID);

/**
  * @}
  */

__VOID __systemInit(__APP_ENTRY* entry);
__VOID __systemProcessTick(__VOID);
__VOID __systemStop(__VOID);
__VOID __systemStart(__VOID);
__VOID __systemEnableScheduler(__VOID);
__VOID __systemDisableScheduler(__VOID);
__VOID __systemScheduleThreadChange(__VOID);
__BOOL __systemSchedulerDisabled(__VOID);
__VOID __systemEnterISR(__VOID);
__VOID __systemLeaveISR(__VOID);
u32 __systemGetTickCount(__VOID);
u32 __systemGetSecondsCount(__VOID);
u32 __systemGetIrqCount(__VOID);


/**
  * @}
  */

#endif
