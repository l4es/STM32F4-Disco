/***************************************************************************
 * rtc.h
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
#ifndef __RTC_H__
#define __RTC_H__

#include <plat_cpu.h>

#if __CONFIG_COMPILE_RTC

/*!
 * @addtogroup Core
 * @{
 */

/*!
 * @defgroup RTC
 * @{
 */

/*!
 * @defgroup RTC_Typedefs Typedefs
 * @{
 */

/*!
 * @brief Date/Time structure.
 */
typedef struct __rtcDateTimeTag
{
	u16 year;		/*!< @brief Year */
	u8 month;		/*!< @brief Month */
	u8 day;			/*!< @brief Day */

	u8 hour;		/*!< @brief Hour */
	u8 min;			/*!< @brief Minute */
	u8 sec;			/*!< @brief Second */

	u8 dayofweek;	/*!< @brief Day of the week */

} __DATETIME, *__PDATETIME;

/**
  * @}
  */

__VOID __rtcInit(__VOID);
__BOOL __rtcSetDateTime(__PDATETIME dt);
__BOOL __rtcGetDateTime(__PDATETIME dt);

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_RTC */

#endif /* __RTC_H__ */
