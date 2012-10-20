/***************************************************************************
 * thlist.c
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
#include "thlist.h"

/** @addtogroup Thread
 * @{
 */

/** @defgroup Thread_ListFunctions Lists functions
  * @brief Functions to manage thread lists.
  *
  * The following functions should be used exclusively by the @ref Core
  * module in order to manage the thread linked lists.
  *
  * Milos scheduler manages three lists by now:
  * -	A list representing the threads ready to run, ordered by priority.
  * 		That is the __threadReady list, defined in thread.c
  * -	A list of suspended threads, ordered by time of arrival (threads
  *  		are appended at the end of the list).
  * -	For each event in the system, a list containing the threads that
  *  		are waiting for that particular event, ordered by priority.
  *
  * The first two methods (__thlAddPrioList()  and __thlRemovePrioList())
  * are used to keep track of the ready threads ordered by priority.
  * __thlAddSuspList()  and __thlRemoveSuspList() manages those threads
  * in suspended (sleeping or waiting) state.
  *
  * Note that the four methods described above use the same linked pointers
  * (@c th_qnext and @c thq_prev). That is because a ready thread cannot be
  * in the suspended list and vice-versa.
  *
  * __thlAddEvtPrio() and __thlRemoveEvtPrio() sort the threads by priority,
  * to be used by __eventSet() and __eventWait().
  *
  * @{
  */


/*!
 * @brief Adds the thread in a priority ordered list.
 *
 * Even this function can be called from outside @ref Core module, it
 * should not.
 *
 * Since we are always removing only the head of the priority ordered
 * list do not use @c th_qprev value when ordering by priority.
 *
 * @param	th		Pointer to a thread pointer.
 * @param 	thb		Pointer to the thread at the head of the list.
 *
 * @return		Nothing.
 *
 */
void __thlAddPrioList(__PTHREAD th, __PTHREAD* thb)
{
	__PTHREAD tl = __NULL;
	__PTHREAD p = *thb;

	/* Set null here, we are doing it anyway ahead in the code
	 * whenever the exit condition.
	 */
	th->th_qnext = th->th_qprev = __NULL;

	if (p == __NULL)
	{
		*thb = th;
		return;
	}

	/*
	 * The worst case here is when N threads equals ALL threads - 1,
	 * and we are adding a thread with the lowest priority
	 */
	while (p && p->th_priority <= th->th_priority)
	{
		tl = p;
		p = p->th_qnext;
	}

	/* Exit conditions:
	 * - 	p is null and is equal to *thb, so
	 * 		th becomes the head of the list (*thb = th).
	 * 		This condition is already checked at the beginning
	 * 		of the function.
	 *
	 * -	p is null means we reached the end of the list.
	 *
	 * -	p is not null and is equal to *thb so
	 * 		th replaces p as the head of the list (*thb = th).
	 *
	 * -	p is not null and is not equal to *thb so th is
	 * 		inserted somewhere in the list.
	 */

	if (p == __NULL)
	{
		tl->th_qnext = th;
	} else
	{
		if (p == *thb)
		{
			th->th_qnext = p;
			*thb = th;
		} else
		{
			th->th_qnext = tl->th_qnext;
			tl->th_qnext = th;
		}
	}
}

/*!
 * @brief Removes the first thread from a priority ordered list.
 *
 * Even this function can be called from outside @ref Core module, it
 * should not.
 *
 * It's a priority ordered list, and this function is called when
 * we are about to execute the thread with highest priority (among those
 * that are ready to run). So is enough to remove only the first thread pointer.
 *
 * @param	thb		Pointer to a thread pointer.
 *
 * @return		Nothing.
 *
 */
void __thlRemovePrioList(__PTHREAD* thb)
{
	__PTHREAD th = *thb;

	*thb = th->th_qnext;
	th->th_qnext = th->th_qprev = __NULL;
}


/*!
 * @brief Adds the thread to the end of the suspended threads list.
 *
 * Even this function can be called from outside @ref Core module, it
 * should not.
 *
 * This is not a priority ordered list. The @c thl parameter points the last
 * thread in the list.
 *
 * @param 	th		Pointer to a thread.
 * @param	thl		Pointer to the last thread pointer in the list.
 *
 * @return		Nothing.
 *
 */
void __thlAddSuspList(__PTHREAD th, __PTHREAD* thl)
{
	th->th_qnext = __NULL;
	th->th_qprev = *thl;
	if (*thl) (*thl)->th_qnext = th;
	*thl = th;
}

/*!
 * @brief Removes the thread from the suspended threads list.
 *
 * Even this function can be called from outside @ref Core module, it
 * should not.
 *
 * This is not a priority ordered list. The @c thl parameter points the last
 * thread in the list.
 *
 * @param 	th		Pointer to a thread.
 * @param	thb		Pointer to the first thread pointer.
 * @param	thl		Pointer to the last thread pointer.
 *
 * @return		Nothing.
 *
 */
void __thlRemoveSuspList(__PTHREAD th, __PTHREAD* thb, __PTHREAD* thl)
{
	/* If the previous in list is not null assign the next in list */
	if (th->th_qprev)
	{
		th->th_qprev->th_qnext = th->th_qnext;
	}

	/* If the next in list is not null assign the previous in list */
	if (th->th_qnext)
	{
		th->th_qnext->th_qprev = th->th_qprev;
	}

	/* If we are removing the head of the list, assign it the next thread */
	if (*thb == th)
	{
		*thb = th->th_qnext;
	}

	/* If the thread we are removing is the last one, assign the previous to thl */
	if (*thl == th)
	{
		*thl = th->th_qprev;
	}

	/* If the head is null, the tail should be null */
	if (*thb == __NULL) *thl = __NULL;

	/* Clean */
	th->th_qnext = th->th_qprev = __NULL;

}

/*!
 * @brief Add the thread to the event's thread list, ordered by priority.
 *
 * Even this function can be called from outside @ref Core module, it
 * should not.
 *
 * Because a thread that waits for an event can be removed from the
 * event's list at any time, we should use th_evprev value to save time,
 * instead of cycling the list in a loop.
 *
 * @param 	th		Pointer to a thread.
 * @param	the		Pointer to the head of the list pointer.
 *
 * @return		Nothing.
 *
 */
void __thlAddEvtPrio(__PTHREAD th, __PTHREAD* the)
{
	__PTHREAD tl = __NULL;
	__PTHREAD p = *the;

	/* Set null here, we are doing it anyway ahead in the code
	 * whenever the exit condition.
	 */
	th->th_evnext = th->th_evprev = __NULL;

	if (p == __NULL)
	{
		*the = th;
		return;
	}

	/*
	 * The worst case here is when N threads equals ALL threads - 1,
	 * and we are adding a thread with the lowest priority
	 */
	while (p && p->th_priority <= th->th_priority)
	{
		tl = p;
		p = p->th_evnext;
	}

	/* Exit conditions:
	 * - 	p is null and is equal to *the, so
	 * 		th becomes the head of the list (*the = th).
	 * 		This condition is already checked at the beginning
	 * 		of the function.
	 *
	 * -	p is null means we reached the end of the list.
	 *
	 * -	p is not null and is equal to *the so
	 * 		th replaces p as the head of the list (*the = th).
	 *
	 * -	p is not null and is not equal to *the so th is
	 * 		inserted somewhere in the list.
	 */

	if (p == __NULL)
	{
		tl->th_evnext = th;
		th->th_evprev = tl;
	} else
	{
		if (p == *the)
		{
			p->th_evprev = th;
			th->th_evnext = p;
			*the = th;
		} else
		{
			th->th_evnext = p;
			tl->th_evnext = p->th_evprev = th;
			th->th_evprev = tl;
		}
	}
}

/*!
 * @brief Removes the thread from the event's thread list, ordered by priority.
 *
 * Even this function can be called from outside @ref Core module, it
 * should not.
 *
 * Because a thread that waits for an event can be removed from the
 * event's list at any time, we should use th_evprev value to save time,
 * instead of cycling the list in a loop.
 *
 * @param 	th		Pointer to a thread.
 * @param	the		Pointer to the head of the list pointer.
 *
 * @return		Nothing.
 *
 */
void __thlRemoveEvtPrio(__PTHREAD th, __PTHREAD* the)
{
	if (th->th_evprev) th->th_evprev->th_evnext = th->th_evnext;
	if (th->th_evnext) th->th_evnext->th_evprev = th->th_evprev;
	if (*the == th) *the = th->th_evnext;
	th->th_evnext = th->th_evprev = __NULL;
}

/**
  * @}
  */

/**
  * @}
  */
