/***************************************************************************
 * queue.c
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

#include "queue.h"
#include "system.h"
#include "heap.h"
#include <common/inc/mem.h>

#if __CONFIG_COMPILE_QUEUE

/** @addtogroup Core
  * @{
  */

/** @defgroup Queue Queue
  * @{
  */

/** @defgroup Queue_PrivateConstants Private constants
  * @{
  */

#define __QUEUE_READY	0x80	/*!< @brief Queue ready flag */

/**
  * @}
  */

/** @defgroup Queue_PrivateMacros Private macros
  * @{
  */


#define __queueReady(x)	(x && (x->type & __QUEUE_READY ? __TRUE : __FALSE))	/*!< @brief Checks if the queue is ready */

/**
  * @}
  */

/** @defgroup Queue_PrivateTypedefs Private typedefs
  * @{
  */

/*!
 * @brief Structure used in dynamically-allocated queue type.
 */
typedef struct queueDataTag {
	u32 datalen;				/*<! @brief Data length */
	__PVOID data;				/*<! @brief Pointer to data */
	struct queueDataTag* next;	/*<! @brief Pointer to next __QUEUE_DATA */
} __QUEUE_DATA, *__PQUEUE_DATA;

/**
  * @}
  */


/** @defgroup Queue_Functions Functions
  * @{
  */


/*!
 * @brief Creates and prepares a queue structure.
 *
 * Call this function before calling any other queue functions.
 * The \c type parameter defines the operating mode: if __QUEUE_ALLOC is set,
 * __heapAlloc() will be used to dynamically create	a memory space to store
 * queue items. Otherwise the user must provide a valid pointer (\c ptr and
 * \c ptr_size parameters).
 *
 * If a \c ptr pointer is provided, the queue automatically is set to operate in fixed-size
 * mode (__QUEUE_FIXED_DATA_SIZE flag), so parameters \c item_qty and \c item_size must be
 * provided \b and \c ptr_size \b must equal (\c item_qty * \c item_size).
 *
 * Dynamic allocation can be set to operate in two modes:
 * to grow automatically (using __heapAlloc()) every time an item is appended, or
 * to create a fixed memory area where all the queue items have a fixed size (using
 * the __QUEUE_FIXED_DATA_SIZE flag).
 *
 * Queues work by copying the data, so the pointers provided in __queueAdd() and __queueGet()
 * can be reused after calling those functions.
 *
 * Users can wait for data using the __queueWaitForData(), that will put the calling thread
 * to sleep.
 *
 * @param	type	Queue type. Bitwise value that determines the operating mode.
 * @arg __QUEUE_STATIC			The user provides a pointer to store the queue items. Using
 * 								this value forces the queue to work with __QUEUE_FIXED_DATA_SIZE
 * 								flag set.
 * @arg	__QUEUE_ALLOC			The queue functions will dynamically allocate memory with __heapAlloc().
 * @arg __QUEUE_FIXED_SIZE		Items in the queue have a fixed size. When used with __QUEUE_ALLOC,
 * 								__queuePrepare() will allocate memory once, for all the possible
 * 								items (determined by the \c item_qty and \c item_size parameters).
 * 								Otherwise memory is allocated each time __queueAdd() is called and
 * 								freed on __queueGet(). The last mode is used when the length of the
 * 								appended items is unknown.
 * @param queue					Pointer to the queue structure to be initializated.
 * @param item_qty				Maximum item quantity in the queue. Used when the \c type parameter
 * 								has the __QUEUE_STATIC and/or the __QUEUE_FIXED_DATA_SIZE flags enabled.
 * @param item_size				Fixed item size. Used when the \c type parameter
 * 								has the __QUEUE_STATIC and/or the __QUEUE_FIXED_DATA_SIZE flags enabled.
 * @param ptr					Pointer to memory space to store queue items. Used when the queue
 * 								is of \c type __QUEUE_STATIC.
 * @param ptr_size				Lenght of \c ptr parameter, if provided.
 *
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __queueCreate(u8 type, __PQUEUE queue, u32 item_qty, u32 item_size, u8* ptr, u32 ptr_size)
{
	if (!queue) return __FALSE;
	if (queue->type & __QUEUE_READY) return __FALSE;

	queue->item_count = 0;
	queue->type = type;
	__memSet(&queue->event_r, 0, sizeof(__EVENT));
	__memSet(&queue->event_w, 0, sizeof(__EVENT));

	/*
	 * Check type.
	 */
	if (queue->type & __QUEUE_ALLOC)
	{
		if (queue->type & __QUEUE_FIXED_SIZE)
		{
			/*
			 * Allocate a fixed size determined by (qty * size)
			 */
			if (!item_size || !item_qty) return __FALSE;

			queue->item_size = item_size;
			queue->item_qty = item_qty;
			queue->data_size = queue->item_qty * queue->item_size;
			queue->ref = __heapAllocZero(queue->data_size);
			if (!queue->ref) return __FALSE;
			queue->data_w  = queue->data_r = queue->ref;
		} else {
			/*
			 * Queue is grown every time an item is appended.
			 */
			queue->data_size = queue->item_qty = queue->item_size = 0;
			queue->ref = queue->data_w = queue->data_r = __NULL;
		}

		queue->type |= __QUEUE_READY;
		return __TRUE;
	} else {
		/*
		 * A data pointer is provided, allocated elsewhere.
		 * In this case we only accept fixed-size items
		 */
		if (!ptr || !ptr_size || !item_qty || !item_size) return __FALSE;

		/*
		 * We will be appending fixed size items on a pre-allocated
		 * memory area. We should check that the ptr provided and it's size
		 * could fit qty of items * size of items.
		 */
		if (ptr_size != (item_qty * item_size)) return __FALSE;

		queue->data_size = ptr_size;
		queue->item_size = item_size;
		queue->item_qty = item_qty;
		queue->ref = queue->data_w = queue->data_r = ptr;
		queue->type = __QUEUE_FIXED_SIZE;
		queue->type |= __QUEUE_READY;

		return __TRUE;
	}
}

/*!
 * @brief Appends an item to a queue.
 *
 * Call this function to append data to the end of the queue. The data is copied
 * so the \c data pointer provided can be reused after calling this function.
 * Only the thread that created the queue can write to it.
 * This function calls __eventSet() to wake up the thread that waits for data.
 * If the queue is of type fixed, \c size parameter must not be greater than the value
 * provided in the __queueCreate() call.
 *
 * @param queue		Pointer to the queue. Has to be initialized calling __queueCreate().
 * @param data		Pointer to data to write.
 * @param size		Size in bytes of the data referenced by the \c data parameter.
 *
 * @return	__TRUE on success, otherwise __FALSE.
 *
 */
__BOOL __queueAdd(__PQUEUE queue, __PVOID data, u32 size)
{
	/*
	 * Queue ready?
	 */
	if (!__queueReady(queue)) return __FALSE;

	/*
	 * TODO Check calling from isr and return __FALSE if so.
	 */

	if (queue->type & __QUEUE_FIXED_SIZE)
	{
		/* Check for right size */
		if (size > queue->item_size) return __FALSE;

		/* Check for items stored */
		if (queue->item_count >= (queue->data_size / queue->item_size))
			return __FALSE;

		/*
		 * This can be shared, so stop scheduler.
		 */
		__systemDisableScheduler();

		/* If the next position to write is at the end of the buffer, set it to the
		 * beginning.
		 */
		if (queue->data_w == ((u8*) queue->ref + queue->data_size)) queue->data_w = queue->ref;

		__memCpy(queue->data_w, data, size);

		queue->data_w = (u8*) queue->data_w + queue->item_size;
		queue->item_count++;

		__systemEnableScheduler();

	} else {
		__PQUEUE_DATA ptr;

		ptr = queue->data_w;
		if (!ptr)
		{
			ptr = __heapAllocZero(sizeof(__QUEUE_DATA) + size);
		} else {
			ptr->next = __heapAllocZero(sizeof(__QUEUE_DATA) + size);
			if (!ptr->next) return __FALSE;
			ptr = ptr->next;
		}

		__systemDisableScheduler();

		queue->data_w = ptr;

		ptr->data = ((u8*) ptr + sizeof(__QUEUE_DATA));
		ptr->datalen = size;
		ptr->next = __NULL;

		__memCpy(ptr->data, data, size);

		queue->item_count++;
		queue->data_size += sizeof(__QUEUE_DATA) + size;

		if (!queue->data_r) queue->data_r = ptr;

		__systemEnableScheduler();
	}

	__eventSet(&queue->event_r);
	return __TRUE;
}

/*!
 * @brief Reads data from the queue.
 *
 * Call this function to retrieve data from the queue.
 * The size of the first element in the queue can be retrieved calling this function
 * with the \c buf parameter set to __NULL (\c len parameter must not be __NULL in any
 * case).
 * If the queue item size is not fixed (created with the __QUEUE_ALLOC flag), calling this
 * function will generate a __heapFree() call in order to release unused item memory.
 *
 */
__BOOL __queueGet(__PQUEUE queue, __PVOID buf, u32* len)
{
	__PVOID ptr;

	/*
	 * Queue ready?
	 */
	if (!__queueReady(queue) || !len) return __FALSE;

	/*
	 * TODO Check calling from isr and return if so.
	 */

	/*
	 * Stored items?
	 */
	if (!__queueGetItemCount(queue)) return __FALSE;

	ptr = queue->data_r;

	/*
	 * If buf is null, user may want to know the size of the first
	 * available item.
	 */
	if (!buf)
	{
		if (queue->type & __QUEUE_FIXED_SIZE)
		{
			*len = queue->item_size;
			return __TRUE;
		}

		*len = ((__PQUEUE_DATA)(ptr))->datalen;
		return __TRUE;
	}

	if (queue->type & __QUEUE_FIXED_SIZE)
	{
		if (*len > queue->item_size) return __FALSE;

		/*
		 * This can be shared, so disable scheduler.
		 */
		__systemDisableScheduler();

		/* If the next position to read is at the end of the buffer, set it to the
		 * beginning.
		 */
		if ((u8*) ptr >= ((u8*) queue->ref + queue->data_size)) ptr = queue->ref;

		__memCpy(buf, ptr, *len);
		*len = queue->item_size;

		/*
		 * Move data_r to the next position.
		 */
		queue->data_r = (u8*) queue->data_r + queue->item_size;

		queue->item_count--;
		__systemEnableScheduler();

		if (!queue->item_count)
			__eventSet(&queue->event_w);

		return __TRUE;
	} else {

		__PQUEUE_DATA qd;

		/*
		 * This can be shared, so disable scheduler.
		 */
		__systemDisableScheduler();

		qd = queue->data_r;

		/*
		 * Check requested size.
		 */
		if (*len < qd->datalen)
		{
			__systemEnableScheduler();
			return __FALSE;
		}

		__memCpy(buf, qd->data, qd->datalen);
		*len = qd->datalen;

		queue->data_r = qd->next;
		queue->item_count--;
		queue->data_size -= sizeof(__QUEUE_DATA) + qd->datalen;

		if (!queue->item_count)
		{
			queue->data_r = queue->data_w = __NULL;
		}

		/*
		 * Deallocate qd
		 */
		__heapFree(qd);

		__systemEnableScheduler();

		if (!queue->item_count)
			__eventSet(&queue->event_w);

		return __TRUE;
	}
}

__BOOL __queueWaitForData(__PQUEUE queue, u32 timeout)
{
	/*
	 * Queue ready?
	 */
	if (!__queueReady(queue)) return __FALSE;

	/*
	 * TODO Check calling from isr and return if so.
	 */

	/*
	 * If already have items, return immediately.
	 */
	if (queue->item_count) return __TRUE;

	/*
	 * Reset event and wait for data.
	 */
	__eventReset(&queue->event_r);
	if (__eventWait(&queue->event_r, timeout) == __EVRET_SUCCESS)
		return __TRUE;

	return __FALSE;
}

__BOOL __queueWaitForEmpty(__PQUEUE queue, u32 timeout)
{
	/*
	 * Queue ready?
	 */
	if (!__queueReady(queue)) return __FALSE;

	/*
	 * TODO Check calling from isr and return if so.
	 */

	/*
	 * If already have items, return immediately.
	 */
	if (!queue->item_count) return __TRUE;

	/*
	 * Reset event and wait for empty queue.
	 */
	__eventReset(&queue->event_w);
	if (__eventWait(&queue->event_w, timeout) == __EVRET_SUCCESS)
		return __TRUE;

	return __FALSE;
}

__BOOL __queueIsReady(__PQUEUE queue)
{
	return __queueReady(queue);
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

#endif /* __CONFIG_COMPILE_QUEUE */
