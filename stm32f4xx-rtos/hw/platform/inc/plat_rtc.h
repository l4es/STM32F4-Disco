/***************************************************************************
 * plat_rtc.h
 * (C) 2011 Ivan Meleca
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
#ifndef __PLAT_RTC_H__
#define __PLAT_RTC_H__

#include "plat_cpu.h"
#include <core/inc/rtc.h>

#if __CONFIG_COMPILE_RTC

/*!
 * @addtogroup RTC
 * @{
 */

/*!
 * @defgroup RTC_Platform Platform-related
 * @{
 */

/** @defgroup RTC_Stm32 STM32
  * @{
  */

__BOOL __rtcPlatInit(__VOID);
__BOOL __rtcPlatGetDateTime(__PDATETIME dt);
__BOOL __rtcPlatSetDateTime(__PDATETIME dt);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_RTC */

#endif /* __PLAT_RTC_H__ */
