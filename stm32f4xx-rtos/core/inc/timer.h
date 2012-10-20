/***************************************************************************
 * timer.h
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

#ifndef	__TIMER_H__
#define	__TIMER_H__

#include <plat_ostypes.h>

/** @addtogroup Timer
  * @{
  */

/** @defgroup Timer_FunctionPrototypes Prototypes
  * @{
  */

typedef	__VOID (__TIMERFUNC)(__PVOID);

/**
  * @}
  */

/** @defgroup Timer_Constants Constants
  * @{
  */

/** @defgroup Timer_TimerTypesDefines Timer types
  * @{
  */

#define	__TM_NORMAL			0x00		/*!< @brief Normal timer */
#define	__TM_ONESHOOT		0x01		/*!< @brief One shot timer */

/**
  * @}
  */

/** @defgroup Timer_TimerStatesDefines Timer states
  * @{
  */

#define	__TM_READY			0x00		/*!< @brief Timer ready to execute */
#define	__TM_INUSE			0x01		/*!< @brief Timer is in use */
#define	__TM_DELETE			0x02		/*!< @brief Marked for deletion */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup Timer_TypeDefs Typedefs
  * @{
  */

typedef struct __timerTag __TIMER, *__PTIMER;

struct __timerTag {

	u8						ti_type;	/*!< @brief Timer type */
	u8						ti_state;	/*!< @brief Timer state */
	u32						ti_time;	/*!< @brief Ticks to reload */
	u32						ti_load;	/*!< @brief Ticks */
	__PVOID					ti_param;	/*!< @brief Timer optional parameter */
	__TIMERFUNC				*ti_func;	/*!< @brief Timer function */
	__PTIMER				ti_next;	/*!< @brief Pointer to next timer */

};

/**
  * @}
  */

__VOID			__timerInit(u32 stksize);
__PTIMER		__timerCreate(u8 type, u32 time, __TIMERFUNC *func, __PVOID param);
__BOOL			__timerDestroy(__PTIMER ti);

/**
  * @}
  */
#endif	//__TIMER_H__
