/***************************************************************************
 * thread.c
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

#include "thread.h"
#include "heap.h"
#include "event.h"
#include <common/inc/common.h>
#include <common/inc/thlist.h>
#include <core/inc/system.h>

/** @addtogroup Core
  * @{
  */

/** @defgroup Thread Threads
  * Thread functions.
  * @{
  */

/** @defgroup Thread_PrivateVariables Private variables
  * @{
  */

/*!< @brief Thread list chain */
__PTHREAD				__threadChain;

/*!< @brief Current running thread */
__VOLATILE __PTHREAD 	__threadCurrent;

/*!< @brief Thread count */
u16						__threadCount;

/*!< @brief Exchange SP area */
__VOLATILE pu32			__threadSp;

__VOLATILE __PTHREAD	__threadReady;
__VOLATILE __PTHREAD 	__threadSusp;
__VOLATILE __PTHREAD	__threadSuspLast;

/*!< @brief Thread available charset */
#define __TH_CHARSET	"0123456789-abcdefghijklmnopqrstuvwxyz_"

/**
  * @}
  */

/** @defgroup Thread_Functions Functions
  * @{
  */

/*!
 * @brief Adds a thread to the head of the of list of suspended threads.
 *
 * Called from __eventWait() and __threadSleep(), with interrupts
 * disabled. Even if this function is not declared as static,
 * avoid calls to this function outside @ref Core module.
 *
 * @return Nothing.
 */
__VOID __threadSuspend(__PTHREAD th, u8 newstate)
{
	th->th_status = newstate;
	__thlAddSuspList(th, (__PTHREAD*) &__threadSuspLast);
	if (!__threadSusp) __threadSusp = __threadSuspLast;
}

/*!
 * @brief Removes a thread from the list suspended threads.
 *
 * Called from __threadProcessTick() and __eventSet(). Avoid user calls to this function.
 *
 * @return Nothing.
 */
__VOID __threadRemoveFromSuspended(__PTHREAD th)
{
	__thlRemoveSuspList(th, (__PTHREAD*) &__threadSusp, (__PTHREAD*) &__threadSuspLast);
}

/*!
 * @brief Adds a thread to the head of the list of threads with the
 * __THST_READY status set.
 *
 * @return Nothing.
 */
__VOID __threadAddToReadyList(__PTHREAD th)
{
	th->th_status = __THSTS_READY;
	__thlAddPrioList(th, (__PTHREAD*) &__threadReady);
}

/*!
 * @brief Removes a thread from the list of threads with the
 * __THST_READY set. It will automatically set __threadReady
 * the value from th->th_rdynext.
 *
 * @return Nothing.
 */
__STATIC __VOID __threadRemoveReadyListHead(__VOID)
{
	__thlRemovePrioList((__PTHREAD*) &__threadReady);
}

/*!
 * @brief Validates the name of the thread.
 *
 * @param	name		Pointer to the thread name.
 * @return	__TRUE if the name is valid, otherwise __FALSE.
 *
 */
#if __CONFIG_COMPILE_STRING
__BOOL	__threadCheckName(__CONST __PSTRING name)
{
	i8		i;

	if (__strLen( name) > __TH_MAXNAMELEN) return(__FALSE);
	
	__strLower((__PSTRING) name);
	for (i = 0; i < __strLen(name); i++) {
		if (__strChr( __TH_CHARSET,*(name + i)) == __NULL) return(__FALSE);
	}
	return(__TRUE);
}
#endif /* __CONFIG_COMPILE_STRING */

/*!
 * @brief Checks for the existence of a thread.
 *
 * @param	th Pointer to a thread.
 * @return	__TRUE if the thread exists, otherwise __FALSE.
 *
 */
__BOOL	__threadFind(__PTHREAD th)
{

	__PTHREAD	tt;
	if (__threadChain == __NULL) return(__FALSE);	/* no threads were created yet */
	tt = __threadChain;
	while (tt && tt != th) tt = (__PTHREAD) tt->th_lstnext;
	if (tt == __NULL) return(__FALSE);				/* thread does not exist */
	return(__TRUE);
}

/*!
 * @brief Finds a thread by it's name.
 *
 * @param	name
 * @return	Pointer to the thread, __NULL if it cannot be found.
 *
 */
#if __CONFIG_COMPILE_STRING
__PTHREAD	__threadFindName(__CONST __PSTRING name)
{
__PTHREAD	th;

	/* Get first thread in the chain */
	th = __threadChain;

	while (th != __NULL) {
		if (__strCmpNoCase((__PSTRING) th->th_name, name) == 0) return(th);
		/* Get the next thread */
		th = (__PTHREAD) th->th_lstnext;
	}
	return(__NULL);
}
#endif /* __CONFIG_COMPILE_STRING */

/*!
 * @brief Creates a new thread.
 *
 * Call this function to create a new thread.
 *
 * @param	name 		Pointer to a string containing the thread name. The name of the thread
 * has to be unique: different threads cannot share the same name.
 * @param	func		Pointer to a __THREADFUNC() function: the thread execution entry point.
 * @param	prio		Thread priority.
 * @param	stack		Stack size in bytes.
 * @param	ticks		Time to live, in System Time Units. Depends on platform's granularity
 * (generally 1 millisecond).
 * @param	param		Optional void pointer to a user-defined value. It can be retrieved
 * with the __threadGetParameter() function.
 * @return	Pointer to the new thread, __NULL on failure. This value can be used with most of the thread functions.
 *
 * @todo	Thread creation flags, to create a thread in suspended mode.
 */
__PTHREAD	__threadCreate(__CONST __PSTRING name, __THREADFUNC *func, u8 prio, u16 stack, u32 ticks, __PVOID param)
{
	__PTHREAD	th;

#if __CONFIG_COMPILE_STRING
	if (__threadFindName(name) != __NULL) return(__NULL);	/* Thread already exist */
#endif /* __CONFIG_COMPILE_STRING */

	if (func == __NULL) return(__NULL);

	/* Stop system */
 	__systemStop();

	/* Thread name not exist. Allocate new thread */
	th = __threadChain;			/* Get first thread */
	while (th != __NULL)
	{	/* Search last thread in the chain */
		th = (__PTHREAD) th->th_lstnext;
	}

	/* Check stack size value */
	if (stack < __TH_MINSTACKSIZE) stack = __TH_MINSTACKSIZE;
	if (ticks < __TH_MINTICKS)     ticks = __TH_MINTICKS;

	/* Allocate thread structure into the heap */
	if ((th = __heapAllocZero(sizeof(__THREAD))) == __NULL) {
		__systemStart();
		return(__NULL);
	}

	/* Allocate stack into the heap */
	if ((th->th_stkptr = __heapAllocZero(stack + 8)) == __NULL)
	{
		__heapFree(th);
		__systemStart();
		return(__NULL);
	}

	/*	We need save the original stack pointer to use in __heapFree() function when the
		thread is destroyed */

	/* Align stack pointer address (platform dependent) */
	th->th_sp = __cpuStackFramePointer(th->th_stkptr, stack);

	/*	All memory allocation succeeded
		Fill the thread struct with appropriate values */

#if __CONFIG_COMPILE_STRING
	__strCpy((__PSTRING) th->th_name, name);		/* Set thread name */
#endif /* __CONFIG_COMPILE_STRING */

	th->th_status	= __THSTS_READY;				/* Thread is in first time status */
	th->th_load		= CPU_MS_TO_TICKS(ticks);		/* Time Of Live for reload */
	th->th_ttl		= th->th_load;					/* Time Of Live */
	th->th_wait		= 0;							/* Waiting time in ticks */
	th->th_stksize	= stack;						/* Stack size */
	th->th_param	= param;						/* User parameter */
	th->th_qprev	= __NULL;
	th->th_qnext	= __NULL;
	th->th_evprev	= __NULL;
	th->th_evnext	= __NULL;

#if __CONFIG_DEBUG_RR
	th->th_priority	= __CONFIG_DEBUG_RR;
#else
	th->th_priority	= prio;
#endif

	/* Sets the stack to in a way it can be unwinded at the first call (platform dependent) */
	th->th_sp = __cpuMakeStackFrame(th->th_sp, (__PVOID) func,param);

	/*	Chain thread in the chained thread list */
	th->th_lstnext = __threadChain;
	__threadChain = th;

	/* Increment thread count */
	++__threadCount;

	/* Add to the list of ready threads */
	__threadAddToReadyList(th);

	/* Check if this new thread has higher priority than the current one.
	 * If so, set it ready and schedule a thread change.
	 */
	if (__threadGetCurrent() && __threadGetCurrent()->th_priority > th->th_priority)
	{
		/*
		 * Check if the current thread disabled the scheduler,
		 * if not, add the current thread to the ready list,
		 * set __threadCurrent to __NULL and set scheduler.
		 */
		if (!__systemSchedulerDisabled())
		{
			__threadAddToReadyList(__threadGetCurrent());
			__threadSetCurrent(__NULL);
			__systemScheduleThreadChange();
		}
	}

	/*	Activate system */
 	__systemStart();

	return(th);
}


/*!
 * @brief Destroys a thread.
 *
 * Flags the thread for destruction.
 *
 * @param	th 		Pointer to the thread to be destroyed.
 * @return	__TRUE on success, otherwise __FALSE.
 *
 */
__BOOL	__threadDestroy(__PTHREAD th)
{
	__PTHREAD	tb = __threadChain;

	__systemStop();
	if (th == __threadChain)
	{
		__systemStart();
		/* we cannot delete the main thread */
		return(__FALSE);
	}

	while (tb)
	{
		if (tb == th)
		{
			th->th_status = __THSTS_DELETE;
			__systemStart();
			return(__TRUE);
		}
		tb = (__PTHREAD) tb->th_lstnext;
	}
	__systemStart();
	return(__FALSE);
}

/*!
 * @brief Threads enters sleep mode.
 *
 * Call this function to put the thread to sleep. It will be waked up when
 * the time provided elapses. Do not call this function with interrupts disabled or
 * from an ISR.
 *
 * @param	ticks	Time in System Time Units, depends on platform implementation
 * 					(usually milliseconds). If zero, it produces a call to __threadYield().
 * @return	Nothing.
 *
 */
__VOID	__threadSleep(u32 ticks)
{
	__PTHREAD th;
	 
	if (!ticks) {
		__threadYield();
		return;
	}

	__systemStop();
	th = __threadGetCurrent();
	th->th_wait = ticks;

	/* Add to suspended threads list */
	__threadSuspend(th, __THSTS_SLEEPING);
	__threadSetCurrent(__NULL);
	__systemScheduleThreadChange();
	__systemStart();

	while (th->th_status != __THSTS_RUNNING);
	return;
}

/*!
 * @brief Changes the time-to-live value of a thread.
 *
 * @param	th		Pointer to a thread.
 * @param	time	Time in System Time Units, depends on platform implementation
 * (usually milliseconds).
 * @return	__TRUE on success, __FALSE otherwise.
 *
 */
__BOOL __threadChangeTime(__PTHREAD th, u32 time)
{
	if (time < (u32) 1) return(__FALSE);
	if (th == __NULL)
	{
		th = __threadGetCurrent();
	} else
	{
		if (__threadFind(th) == __FALSE) return(__FALSE);
	}
	th->th_ttl = th->th_load = time;

	return(__TRUE);
}

/*!
 * @brief Retrieves the thread parameter.
 *
 * Call this function to get the parameter set at the time of the thread creation.
 * See __threadCreate() function.
 *
 * @return	The thread parameter pointer.
 *
 */
__PVOID	__threadGetParameter(__VOID)
{
	__PTHREAD th = __threadGetCurrent();

	if (th)
		return((__PVOID) th->th_param);

	return __NULL;
}

/*!
 * @brief Manages threads timing.
 *
 * Called from the main system timer to decrement the time counter of
 * slept/waiting threads. This function also sets preemption and call
 * context switch function if necessary.
 * Avoid user calls to this function.
 *
 * @return	Nothing.
 *
 * @todo To be heavily optimized.
 *
 */
__VOID __threadProcessTick(__VOID)
{
	__PTHREAD tn, th;
	__BOOL preempt = __FALSE;

	__systemStop();

	/* Check time-to-live for the current thread */
	if (__threadGetCurrent())
	{
		/* Decrement thread time-to-live */
		if (--__threadGetCurrent()->th_ttl == 0) {
			/* The current thread should be no longer in execution, set scheduler */
			preempt = __TRUE;
		}
	}
	
	th = tn = __threadSusp;	
	while (tn)
	{
		th = tn;
		tn = tn->th_qnext;

		if (th->th_wait && --th->th_wait == 0)
		{
			__threadRemoveFromSuspended(th);
			__threadAddToReadyList(th);

			/* If the thread we just woke up has a higher priority than the current thread
			 * then we need to preempt. If the current thread is __NULL, there is no thread in
			 * execution, so preempt.
			 */
			if (!preempt)
			{
				if (__threadGetCurrent() == __NULL)
				{
					preempt = __TRUE;
				} else
				{
					if (th->th_priority < __threadGetCurrent()->th_priority) preempt = __TRUE;
				}
			}
		}
	}

	/*
	 * Check if someone has disabled the scheduler.
	 * Thread change can happen only with the scheduler enabled.
	 * A thread change will happen as soon as the scheduler is enabled again.
	 */
	if (!__systemSchedulerDisabled()) {
		if (preempt)
		{
			if (__threadGetCurrent())
			{
				__threadAddToReadyList(__threadGetCurrent());
				__threadSetCurrent(__NULL);
			}

			__systemScheduleThreadChange();
		}
	}
	
	__systemStart();
}

#if (__CONFIG_POST_STACK_OVERFLOW == 1) || defined(__DOXYGEN__)
/*!
 * @brief Post stack overflow detect
 *
 * Called from __threadChange() (and so from an interrupt) when the next running
 * thread stack overflowed on the previous thread change (when all the registers
 * were pushed).
 *
 * Available only if __CONFIG_POST_STACK_OVERFLOW = 1.
 *
 * @param th	The thread with the overflowed stack.
 * @return	Nothing.
 *
 */
__VOID __threadPostStackOverflow(__PTHREAD th)
{

}
#endif /* __CONFIG_POST_STACK_OVERFLOW */

/*!
 * @brief Thread change routine.
 *
 * Main function for context change. Checks the threads states, switches threads contexts and
 * stack pointer. Called from the context change function (usually an ISR).
 *
 * @return	Nothing.
 *
 */
__VOID __threadChange(__VOID)
{
	if (!__threadReady)
	{
		/* going idle */
		__threadSetCurrent(__NULL);
		__cpuIdleIn();
		return;
	}

#if __CONFIG_POST_STACK_OVERFLOW
	if (__threadReady->th_sp < (u32) __threadReady->th_stkptr)
	{
		__threadPostStackOverflow();
	}
#endif /* #ifdef __CONFIG_POST_STACK_OVERFLOW */

	__threadSetCurrent(__threadReady);
	__threadSp = &__threadReady->th_sp;
	__threadReady->th_ttl = __threadReady->th_load;
	__threadReady->th_status = __THSTS_RUNNING;

	__threadRemoveReadyListHead();

	__cpuIdleOut();

	/* Check for threads to delete, if it is not the current one. */
	// if (__threadCurrent->th_status != __THSTS_DELETE)
	//	__threadMaintenance();

#if __PLATCONFIG_CHECK_CS_REENTRY
	/*
	 * Can happen that the interrupt calling this functions is not
	 * atomic at all, and an external IRQ can be setting the Scheduler
	 * to be executed again EVEN if we are still here. So check for the
	 * reentrancy condition and disable the (pending) scheduler.
	 *
	 * An example is the Cortex M3: the __pcd_PendSVHandler instruction
	 * to disable interrupts (CPSID	I) can be anticipated by an external
	 * external interrupt that, for example, sets an event with __eventSet.
	 * If the conditions are met, the __eventSet function can pend the
	 * scheduler again. The result is that the scheduler is executed twice.
	 */
	if (__cpuThreadChangeScheduled())
	{
		__cpuClearPendingThreadChange();
	}
#endif

}

/*!
 * @brief Disables thread execution. Enable with __threadEnable() function.
 *
 * @param 	th		Thread to disable.
 * @return	Nothing.
 *
 */
__VOID	__threadDisable(__PTHREAD th)
{
	th->th_status = __THSTS_DISABLED;	
}

/*!
 * @brief Resumes thread execution.
 *
 * @param 	th		Thread to enable.
 * @return	Nothing.
 *
 */
__VOID __threadEnable(__PTHREAD th)
{
	if (th->th_status == __THSTS_DISABLED)
	{
		th->th_status = __THSTS_READY;
	}
}

/*!
 * @brief Retrieves the threads count.
 *
 * @return	Quantity of threads created.
 *
 */
u16 __threadGetCount(__VOID)
{
	return __threadCount;
}

/*!
 * @brief Retrieves the threads list root.
 *
 * @return	Pointer to the root of the threads linked list.
 *
 */
__PTHREAD	__threadGetChain(__VOID)
{
	return __threadChain;
}

/*!
 * @brief Maintenance tasks.
 *
 * Checks for threads to destroy.
 *
 * @return	Nothing.
 *
 * @todo Call this function when OS is IDLE.
 * Assert when the thread to be destroyed has locks owned or is waiting
 * for an event.
 *
 */
__VOID __threadMaintenance(__VOID)
{
	__PTHREAD th = __threadChain;
	__PTHREAD last = __NULL;
	__PTHREAD next = __NULL;

	while (th)
	{
		if (th->th_status == __THSTS_DELETE)
		{
			if (!last)
			{
				/* Something must be wrong. If last_th is __NULL means
				 * we are trying to delete the system thread.
				 */
			} else
			{
				/* Free stack */
				__heapFree(th->th_stkptr);

				/* Get the next thread in chain */
				next = (__PTHREAD) th->th_lstnext;

				/* Free __PTHREAD */
				__heapFree(th);

				/* Link chain */
				last->th_lstnext = next;

				__threadCount--;
			}
		}

		/* Remember last thread in chain */
		last = th;

		th = (__PTHREAD) th->th_lstnext;
	}
}

/*!
 * @brief Pauses temporarily the thread execution allowing other threads
 * (with higher or equal priority) to run.
 *
 * Call this function to invoke the scheduler to check on higher or equal priority
 * threads to run. If so, the current (calling) thread will be queued in the
 * thread-ready list. If there are not threads with higher or equal priority, this
 * function returns.
 *
 * Do not call this function with interrupts and/or scheduler disabled. The call must
 * be made from within a thread.
 *
 * @return Nothing.
 *
 */
__VOID __threadYield(__VOID)
{

	__systemStop();

	/* Check if there are ready threads with higher or equal priority.
	 * In fact, for most cases, the first ready-thread has to be a thread
	 * with the same priority of the current one. If a thread with higher
	 * priority is ready-to-run, IT SHOULD BE RUNNING.
	 */
	if (__threadGetReady() &&
		__threadGetCurrent()->th_priority >= __threadGetReady()->th_priority)

	{
		/* Add the current thread to the ready list */
		__threadAddToReadyList(__threadGetCurrent());

		/* We have no thread in execution */
		__threadSetCurrent(__NULL);

		/* Schedule context switch */
		__systemScheduleThreadChange();
	}

	__systemStart();
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
