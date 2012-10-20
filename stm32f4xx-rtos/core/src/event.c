/***************************************************************************
 * event.c
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

#include "event.h"
#include "system.h"
#include <common/inc/thlist.h>

/** @addtogroup Core
  * @{
  */

/** @defgroup Event Events
  * @brief Event functions.
  *
  * This module defines the common code for events.
  * Event are used to signal the occurrence of an event from an ISR to a thread, or from a thread
  * to another thread. A thread can wait only for a single event, and many threads can wait for
  * the same event. Events are heavily used, for example in @ref Lock and @ref Queue.
  *
  * @{
  */

/** @defgroup Event_Functions Functions
  * @{
  */

/*!
 * @brief 	Puts a thread in the event's queue and sets it
 * 			into waiting state.
 *
 * Queues the thread in the event's thread list, ordered by
 * priority. Puts the thread into waiting state.
 *
 * @param 		ev		Pointer to an event.
 * @param		th		Pointer to a thread.
 * @param		timeout	Timeout value for the __threadSuspend
 * 						function.
 *
 * @return		Nothing.
 *
 */
__STATIC __VOID __eventQueueThread(__PEVENT ev, __PTHREAD th, u32 timeout)
{

	/* This function is called with disabled interrupts */

	/* Suspend the thread */
	th->th_wait = CPU_MS_TO_TICKS(timeout);
	__threadSuspend(th, __THSTS_WAITING);

	/* Insert the thread in the list */
	__thlAddEvtPrio(th, &ev->ev_threads);
}

/*!
 * @brief 	Removes the thread from the event's thread list.
 *
 * @param 		ev		Pointer to an event.
 * @param		th		Pointer to a thread.
 *
 * @return		Nothing.
 *
 */
__STATIC __VOID __eventDequeueThread(__PEVENT ev, __PTHREAD th)
{
	__thlRemoveEvtPrio(th, &ev->ev_threads);
}

/*!
 * @brief Signals an event with a return code.
 * @param ev	Pointer to event.
 * @param val	Value to return.
 * @return Nothing.
 */
__STATIC __VOID __eventSignal(__PEVENT ev, u8 val)
{
	__PTHREAD th;

	/* Signal the event */
	ev->ev_state = val;

	/* wake up the thread
	 * (if any, we can be setting an event with no one waiting for it)
	 */
	if (ev->ev_threads)
	{
		/* Disable interrupts */
		__systemStop();

		/* wake up each thread, schedule if one of these threads
		 * has a higher priority than the current (running) one.
		 * The worst case here is when ALL threads, except the running
		 * one are waiting for this event.
		 */

		/* If the first thread of the list has a higher priority of the current
		 * one is enough to preempt (the list is ordered by priority)
		 */
		if (__threadGetCurrent())
		{
			if (__threadGetCurrent()->th_priority > ev->ev_threads->th_priority)
			{
				/*
				 * Check if the current thread disabled the scheduler,
				 * if not, add the current thread to the ready list,
				 * __threadCurrent to __NULL and set scheduler.
				 */
				if (!__systemSchedulerDisabled())
				{

					/* Add the current thread to the ready list */
					__threadAddToReadyList(__threadGetCurrent());

					/* We have no thread in execution */
					__threadSetCurrent(__NULL);
				}

				/* Schedule context switch */
				__systemScheduleThreadChange();
			}
		} else {

			/* No thread running
			 * Schedule context switch anyway
			 */
			__systemScheduleThreadChange();
		}

		/* Scan the event's thread list */
		th = ev->ev_threads;
		while (th)
		{
			/* Here we will check if the thread is ready.
			 * Can happen that a READY thread that went through timeout
			 * it is being set as READY again and removed from suspended
			 * and REINSERTED in the ready list TWICE.
			 */
			if (th->th_status == __THSTS_WAITING)
			{
				__threadRemoveFromSuspended(th);
				__threadAddToReadyList(th);
			}

			th = th->th_evnext;
		}

		__systemStart();
	}
}

/*!
 * @brief 	Sets the thread into suspended mode state waiting
 * 			for an event.
 *
 * Call this function to wait for an event. Do not call with context switch
 * disabled. Call this function from a thread.
 *
 * @param 		ev		Pointer to an event to wait for.
 * @param		timeout	Maximum time to wait for the event. Zero for infinite.
 * @return		The following values:
 * @arg __EVRET_ERROR: on error.
 * @arg __EVRET_SUCCESS: event triggered.
 * @arg __EVRET_TIMEOUT:event timeout.
 */
u8	__eventWait(__PEVENT ev, u32 timeout)
{
	u8 ret = 0;
	u32 irqs = 0;
	__PTHREAD th;

	/* Arrived here with interrupts disabled? */
	irqs = __systemGetIrqCount();
	if (!irqs)
	{
		/* Go critical */
		__systemStop();
	} else {
		/* Remove all pending irq enabled.
		 * The next call to __systemStart (before switching context)
		 * SHOULD really enable interrupts. So we save the interrupt-disabled
		 * counter in irqs, to re-establish the __systemIrqCount counter on function
		 * exit.
		 */
		while (__systemGetIrqCount() != 1) __systemStart();
	}

	/* Check if the event is already set */
	if (ev->ev_state != __EV_RESET)
	{
		/*	Means the event was set before entering this function.
			That's why the __eventReset() function: it has to be called before waiting, because the event
			may be set before the call. So return the event was successfully signaled. */

		/* should return with disabled irq? */
		if (!irqs) {
			__systemStart();
		}
		return(__EVRET_SUCCESS);
	}

	th = __threadGetCurrent();

	/* Queue the current thread in the event's thread list */
	__eventQueueThread(ev, th, timeout);

	/* From now, we have no thread in execution */
	__threadSetCurrent(__NULL);

	/* Schedule context switch */
	__systemScheduleThreadChange();

	/* Enable interrupts */
	__systemStart();

	while (th->th_status != __THSTS_RUNNING);
	
	/* At this point the thread resumes. Test for state and
	 * eventually timeout.
	 */

	/* Critical again */
	__systemStop();

	/* Remove the current thread from the event's thread list */
	__eventDequeueThread(ev, th);

	/* should return with enabled irq? */
	if (irqs == 0) {
		__systemStart();
	} else {
		/* re-establish __systemIrqCount counter */
		while (__systemGetIrqCount() != irqs) __systemStop();
	}

	/* Check for timeout */
	if (timeout && !th->th_wait)
	{
		ret = __EVRET_TIMEOUT;
	} else
	{
		ret = __EVRET_SUCCESS;
	}

	if (ev->ev_state == __EV_ABORT)
	{
		ret = __EVRET_ABORT;
	}

	return ret;
}

/*!
 * @brief Signals the event. Wakes up ALL the waiting threads.
 *
 * Call this function to toggle the event to __EV_SET state, waking up
 * a waiting thread.
 *
 * @param	ev		Pointer to event.
 * @return Nothing.
 */
__VOID	__eventSet(__PEVENT ev)
{
	__eventSignal(ev, __EV_SET);
}


/*!
 * @brief Signals the event. Wakes up the first waiting thread.
 *
 * Call this function to toggle the event to __EV_SET state, waking up
 * a waiting thread. Do not call with context switch disabled.
 *
 * @param	ev		Pointer to event.
 * @return Nothing.
 */
__VOID __eventSetOne(__PEVENT ev)
{
	__systemStop();

	ev->ev_state = __EV_SET;

	/* wake up the thread
	 * (if any, we can be setting an event with no one waiting for it)
	 */
	if (ev->ev_threads)
	{
		if (__threadGetCurrent())
		{
			if (__threadGetCurrent()->th_priority > ev->ev_threads->th_priority)
			{
				/*
				 * Check if the current thread disabled the scheduler,
				 * if not, add the current thread to the ready list,
				 * __threadCurrent to __NULL and set scheduler.
				 */
				if (!__systemSchedulerDisabled())
				{
					__threadAddToReadyList(__threadGetCurrent());
					__threadSetCurrent(__NULL);
					__systemScheduleThreadChange();
				}
			}
		} else {
			__systemScheduleThreadChange();
		}

		if (ev->ev_threads->th_status == __THSTS_WAITING)
		{
			__threadRemoveFromSuspended(ev->ev_threads);
			__threadAddToReadyList(ev->ev_threads);
		}

		__thlRemoveEvtPrio(ev->ev_threads, (__PTHREAD*) &ev->ev_threads);
	}

	__systemStart();
}

/*!
 * @brief Sets the event to __EV_RESET state.
 *
 * Prepares the event to be used with the __eventWait() function.
 * @param	ev		Pointer to event.
 * @return Nothing.
 */
__VOID __eventReset(__PEVENT ev)
{
	__systemStop();
	ev->ev_state = __EV_RESET;
	__systemStart();
}

/*!
 * @brief Signals the event with the __EV_ABORT value.
 *
 * This function will Wake up all the waiting threads with the abort signal.
 *
 * Call this function before deleting an event, if there are threads
 * waiting for any signal
 *
 * @param ev	Pointer to event.
 *
 * @return Nothing.
 */
__VOID __eventAbort(__PEVENT ev)
{
	__eventSignal(ev, __EV_ABORT);
}

/*!
 * @brief Returns the list of the threads waiting for the event.
 *
 * @param ev	Pointer to event.
 *
 * @return A pointer to __PTHREAD. The list can be explored following the \c th_evprev and
 * \c th_evnext members.
 */
__PTHREAD __eventGetWaitingThreads(__PEVENT ev)
{
	return ev->ev_threads;
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
