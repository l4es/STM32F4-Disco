/***************************************************************************
 * timer.c
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

#include "timer.h"
#include "thread.h"
#include "heap.h"
#include <core/inc/system.h>

/** @addtogroup Core
  * @{
  */

/** @defgroup Timer Timers
  * Timer functions.
  *
  * All the timers will be called from a single thread. There is a list of callback functions that will
  * be called from __timerDispatch() function. Note that these timers will not be as accurate as a hardware
  * timer: it depends on the system tick granularity, the count down value will be calculated in __systemSleep(1)
  * steps, so it also depends on the __timerThread() thread priority.
  * @{
  */

/** @defgroup Timer_PrivateVariables Private variables
  * @{
  */

__PTIMER		__timerChain = __NULL;		/*!< @brief Timer list chain */
u32				__timerStack = 0;			/*!< @brief Timer stack size */
__PTHREAD		__timerThreadPtr = __NULL;	/*!< @brief Timer thread */

/**
  * @}
  */

/** @defgroup Timer_Functions Functions
  * @{
  */

/*!
 * @brief Finds a given timer through the list.
 *
 * @param ti	Pointer to a timer handle to verify.
 * @return __TRUE if the timer was found in the list, otherwise __FALSE.
 *
 */
__STATIC __BOOL __timerFind(__PTIMER ti)
{
	__PTIMER	tr = __timerChain;

	if (ti == __NULL) return(__FALSE);
	while (tr) {
		if (tr == ti) return(__TRUE);
		tr = (__PTIMER) tr->ti_next;
	}
	return(__FALSE);
}

/*!
 * @brief Creates and allocates a timer and adds it to the __timerChain list.
 *
 * When the time provided elapses, the __timerDispatch() function will call the
 * __TIMERFUNC() function.
 *
 * @param type	Timer type.
 * @arg	__TM_NORMAL: normal timer.
 * @arg	__TM_ONESHOT: one-shot. Timer will be deleted after the first execution.
 * @param time	Time-out value, in system ticks units (ussualy milliseconds).
 * @param func	Pointer to a __TIMERFUNC() function.
 * @param param	Optional parameter to be passed to the __TIMERFUNC() function.
 * @return A timer handle on success, otherwise __NULL.
 *
 */
__PTIMER __timerCreate(u8 type, u32 time, __TIMERFUNC *func, __PVOID param)
{
	__PTIMER	ti;

	if (time < 1 || func == __NULL || type > 1) return(__NULL);
	if ((ti = __heapAlloc(sizeof(__TIMER))) == __NULL) return(__NULL);

	ti->ti_type					= type;
	ti->ti_state				= __TM_READY;
	ti->ti_time = ti->ti_load	= time;
	ti->ti_param				= param;
	ti->ti_func					= func;

	/* Chain to list */
	__systemStop();
	ti->ti_next = __timerChain;
	__timerChain = ti;
	__systemStart();

	return(ti);
}

/*!
 * @brief Destroy a timer.
 *
 * Call this function to destroy a timer. The timer is not immediately
 * destroyed, but flagged to do so. The __timerMaintenance() function
 * (called from the timers thread __timerThread() function)
 * will delete all flagged timers.
 *
 * @param	ti		Pointer to a timer to destroy.
 * @return	__TRUE on success, otherwise __FALSE.
 *
 */
__BOOL __timerDestroy(__PTIMER ti)
{
	if (__timerFind(ti) == __TRUE)
	{
		ti->ti_type |= __TM_DELETE;
		return(__TRUE);
	}
	return(__FALSE);
}

/*!
 * @brief Timer dispatching.
 *
 * This function will be called from the timers thread (__timerThread() function).
 * It will check if the timer has gone into time-out and will call the registered
 * __TIMERFUNC() function. Avoid calls to this function.
 *
 * @return Nothing.
 *
 */
__VOID __timerDispatch(__VOID)
{
	__PTIMER	ti = __timerChain;
	__PTIMER	tb = __NULL;

	/* Cycle through timers */
	while (ti)
	{
		/* Ready? */
		if (ti->ti_state == __TM_READY)
		{
			/* Timeout? */
			if (--ti->ti_time == 0)
			{
				ti->ti_state = __TM_INUSE;
				/* Call provided function */
				(*ti->ti_func)(ti->ti_param);
				if (ti->ti_type == __TM_ONESHOOT)
				{
					/* Delete */
					ti->ti_state |= __TM_DELETE;
				} else
				{
					/* Refresh time-out value */
					ti->ti_time = ti->ti_load;
					ti->ti_state = __TM_READY;
				}
			}
		}

		/* Flagged to delete? */
		if (ti->ti_state == __TM_DELETE)
		{
			if (!tb) {
				__systemStop();
				__timerChain = __NULL;
				__systemStart();
				__heapFree(ti);
				return;
			} else
			{
				tb->ti_next = ti->ti_next;
				__heapFree(ti);
				ti = tb->ti_next;
			}
		} else {
			tb = ti;
			ti = (__PTIMER) ti->ti_next;
		}
	}
}

/*!
 * @brief Timers thread.
 *
 * Timers thread function. Will maintain and poll every timer created with the
 * __timerCreate() function.
 *
 * @return Nothing.
 *
 */
__VOID __timerThread(__VOID)
{
	for(;;)
	{
		__timerDispatch();
		__threadSleep(1);
	}

}

/*!
 * @brief Timers initialization.
 *
 * Called from __systemInit() function to create the thread that will manage
 * the timers.
 *
 * @param stksize	Size of the timers thread stack.
 * @return Nothing.
 *
 */

__VOID __timerInit(u32 stksize)
{
	if (!__timerThreadPtr)
	{
		__timerStack = stksize;
		__timerThreadPtr = __threadCreate("timer", __timerThread, __CONFIG_PRIO_TIMTHREAD, __timerStack, 1, __NULL);
	}
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
