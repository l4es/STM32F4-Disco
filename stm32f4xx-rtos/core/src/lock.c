/***************************************************************************
 * lock.c
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

#include "lock.h"
#include "heap.h"
#include "dbgterm.h"
#include <common/inc/common.h>
#include <core/inc/system.h>

/** @addtogroup Core
  * @{
  */

/** @defgroup Lock Locks
  * Lock functions.
  *
  * This module defines the locks functions.
  * Locks can be seen as mutexes: one thread requesting access to a resource may
  * call __lockOwn() and release the resource with __lockRelease(). Locks will enqueue
  * the threads waiting for a resource (lock) in a priority-ordered queue, so a high-priority
  * thread will be awakened first when the resource is freed.
  * A timeout value can be passed on __lockOwn() to prevent deadlocks. This is, obviously, not
  * enough and in some cases priority-inheritance or priority-ceiling algorithms could come handy
  * (to be implemented).
  *
  * @{
  */

/** @defgroup Lock_PrivateVariables Private variables
  * @{
  */

u8 __lockCount = 0;	/*!< @brief Locks count. */

/**
  * @}
  */


/** @addtogroup Lock_PrivateVariables
  * @{
  */

__PLOCK_LIST __lockList = __NULL;	/*!< @brief List for terminal walk command & maintenance. */

/**
  * @}
  */

/** @defgroup Lock_Functions Functions
  * @{
  */

/*!
 * @brief Creates a lock.
 *
 * Call this function to allocate a lock. Do not free the pointer returned, call
 * __lockDestroy() function instead. A lock can always be created without calling
 * this function, but it will not be managed by the __lockList list.
 * @return	A pointer to a lock.
 */
__PLOCK __lockCreate(__VOID)
{
__PLOCK			lock;
__PLOCK_LIST	list;
__PLOCK_LIST	list_root = __NULL;

	__systemStop();

	/* Alloc and assign */
	lock = __heapAlloc(sizeof(__LOCK));
	if (!lock)
	{
		__systemStart();
		return __NULL;
	}

	lock->state 	= __LOCK_RELEASED;
	lock->type		= __LOCK_TYPE_NORMAL;
	lock->owner		= __NULL;
	lock->parent	= __threadGetCurrent();
	
	/* Add to list */
	list = __lockList;
	while(list != __NULL)
	{
		/* Remember last list */
		list_root = list;
		list = list->next;
	}

	list = __heapAllocZero(sizeof(__LOCK_LIST));
	if (!list)
	{
		__heapFree(lock);
		__systemStart();
		return __NULL;
	}

	list->lock = lock;
	list->next = __NULL;

	if (__lockList == __NULL)
	{
		__lockList = list;
	} else
	{
		list_root->next = list;
	}

	__lockCount++;

	__systemStart();

	return lock;
}

/*!
 * @brief Puts the calling thread to wait to the lock to be released.
 *
 * Internal function called from __lockOwn() function. After waiting for the
 * lock, the thread will be removed from the lock's priority list.
 * @param lock	Pointer to a lock.
 * @param wait	Time to wait in milliseconds.
 * @return __LOCK_WAIT_RELEASED if the lock has been released before the time
 * expires. __LOCK_WAIT_TIMEOUT on timeout.
 */
__STATIC u8 __lockWait(__PLOCK lock, u32 wait)
{
	u8 ret = __EVRET_TIMEOUT;

	__eventReset(&lock->event);
	ret = __eventWait(&lock->event, wait);
	return ret;
}

/*!
 * @brief Claims a lock. If it is not released, it will put the calling thread
 * into waiting state.
 *
 * Call this function to claim a lock. If it is not released it will put the
 * calling thread into waiting state until released or the time elapses.
 * The thread will be added to the lock's priority list and waked up in order
 * of arrival. See the __lockAddToPriorityList() function.
 * @param	lock	Pointer to a lock.
 * @param	wait	Time to wait in milliseconds.
 * @return	__TRUE on success claiming the lock, __FALSE otherwise.
 */
__BOOL	__lockOwn(__PLOCK lock, u32 wait)
{
	u32 time = 0;

	__systemStop();

	/* Check if __lockOwn is not being called from an already
	 * "owner" thread.
	 */
	if (lock->state == __LOCK_LOCKED && lock->owner == __threadGetCurrent()) {
		__systemStart();
		return __TRUE;
	}

	/* Here, in the case of a failed lock wait, we will
	 * reuse the time left to wait again.
	 * For example can happen that:
	 *
	 * 1-	Thread1 and Thread2 has the same priority, so they won't preempt
	 * 		each other.
	 * 2- 	Thread1 owns a lock, and goes suspended.
	 * 3- 	Thread2 wants to own the locked lock, but the lock
	 * 		is busy, so it will enter __lockWait().
	 * 4-	Thread1 wakes up, and releases the lock (__lockRelease()),
	 * 		setting Thread2 ready to run. But soon after releasing, Thread1 owns the lock again,
	 * 		and goes suspended.
	 * 5-	Thread2, waked up and ready to run by the __lockRelease() call (from Thread1)
	 * 		tries to own the lock, but it founds it locked.
	 *
	 */
	time = wait;
	while (lock->state != __LOCK_RELEASED && time)
	{
		__lockWait(lock, time);
		time = __threadGetCurrent()->th_wait;
	}

	if (lock->state != __LOCK_RELEASED)
	{
		__systemStart();
		return __FALSE;
	}

	lock->state = __LOCK_LOCKED;
	lock->owner	= __threadGetCurrent();

	__systemStart();
	return __TRUE;
}

/*!
 * @brief Releases a lock.
 *
 * Call this function to release a previously locked event. It will wake up
 * the first thread in the lock's priority list.
 * @param	lock	Pointer to a lock.
 * @return	__TRUE on success, otherwise __FALSE.
 */
__BOOL	__lockRelease(__PLOCK lock)
{
	if (!lock) return __FALSE;

	__systemStop();
	lock->state = __LOCK_RELEASED;
	lock->owner = __NULL;
	
	/* wake threads waiting for this lock */
	__eventSetOne(&lock->event);
	__systemStart();

	return __TRUE;
}

/*!
 * @brief Destroys a lock.
 *
 * Call this function to destroy a lock created with the __lockCreate()
 * function. It will be removed from the __lockList list.
 * @param	lock	Pointer to a lock.
 * @return	Nothing.
 */
__VOID	__lockDestroy(__PLOCK lock)
{
__PLOCK_LIST list 		= __NULL;
__PLOCK_LIST root_list 	= __NULL;

	list = __lockList;
	root_list = list;

	__systemStop();
	while (list)
	{
		if (list->lock == lock)
		{
			__heapFree(lock);

			if (__lockList == list)
			{
				(list->next != __NULL) ? (__lockList = list->next) : (__lockList = __NULL);
			} else
			{
				(list->next != __NULL) ? (root_list->next = list->next) : (root_list->next = __NULL);
			}

			__heapFree(list);

			__lockCount--;

			__systemStart();
			return;
		}

		root_list = list;
		list = list->next;
	}
	__systemStart();
	return;
}

/*!
 * @brief Used to retrieve the total lock count.
 *
 * @return The active lock count.
 */
u32	__lockGetCount(__VOID)
{
	return __lockCount;
}

/*!
 * @brief Returns the list of active locks.
 *
 * @return The list of active locks.
 */
__PLOCK_LIST __lockGetList(__VOID)
{
	return __lockList;
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
