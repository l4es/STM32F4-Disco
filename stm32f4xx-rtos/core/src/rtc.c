/***************************************************************************
 * rtc.c
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

#include "rtc.h"
#include <plat_rtc.h>

#if __CONFIG_COMPILE_RTC

/*!
 * @addtogroup RTC
 * @{
 */

/*!
 * @defgroup RTC_PrivateVariables Private variables
 * @{
 */

__STATIC __BOOL __rtcInitialized = __FALSE;	/*!< @brief Initialization status */

/**
  * @}
  */

/*!
 * @defgroup RTC_Functions Functions
 * @{
 */

/*!
 * @brief Initializes the RTC.
 *
 * Called from __systemInit() to initialize the RTC.
 *
 * @return Nothing.
 */
__VOID __rtcInit(__VOID)
{
	if (__rtcInitialized) return;

	__rtcInitialized = __rtcPlatInit();
}

/*!
 * @brief Sets the current date and time.
 *
 * @param dt	Pointer to a __DATETIME structure containing the date and the time.
 *
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __rtcSetDateTime(__PDATETIME dt)
{
	if (!__rtcInitialized || !dt) return __FALSE;

	return __rtcPlatSetDateTime(dt);
}

/*!
 * @brief Gets the current date and time.
 *
 * @param dt	Pointer to a __DATETIME structure to receive the current date and the time.
 *
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __rtcGetDateTime(__PDATETIME dt)
{
	if (!__rtcInitialized || !dt) return __FALSE;

	return __rtcPlatGetDateTime(dt);
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_RTC */
