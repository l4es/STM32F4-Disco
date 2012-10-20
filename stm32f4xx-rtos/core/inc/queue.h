/***************************************************************************
 * queue.h
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
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <plat_ostypes.h>
#include "thread.h"
#include "event.h"

#if __CONFIG_COMPILE_QUEUE

/** @addtogroup Queue Queue
  * @{
  */

/** @defgroup Queue_Constants Constants
  * @{
  */

/*!
 * @brief Bitwise values for queue type.
 * Bit 0: 	If 1 data is allocated dynamically. Otherwise a pointer must be provided
 * Bit 1: 	If 1 item data in queue has a fixed size, otherwise has a dynamic size.
 */
#define __QUEUE_STATIC			0
#define __QUEUE_ALLOC			1
#define __QUEUE_FIXED_SIZE		2

/**
  * @}
  */

/** @defgroup Queue_Typedefs Typedefs
  * @{
  */

/*!
 * @brief Main queue structure.
 */
typedef struct queueTag {
	__PTHREAD parent;			/*<! @brief Queue creator (has write permission) */
	__EVENT	event_r;			/*<! @brief Event for waiting for data */
	__EVENT	event_w;			/*<! @brief Event for waiting for empty */
	u8 type;					/*<! @brief Queue type */
	u32 item_count;				/*<! @brief Quantity of items in queue */
	u32	data_size;				/*<! @brief Size of allocated data for the queue */
	u32 item_size;				/*<! @brief Size of each item in the queue (if fixed size) */
	u32 item_qty;				/*<! @brief Maximum quantity of items */
	__PVOID data_w;				/*<! @brief Pointer to first-to-write */
	__PVOID data_r;				/*<! @brief Pointer to first-to-read */
	__PVOID ref;				/*<! @brief Base address */
} __QUEUE, *__PQUEUE;

/**
  * @}
  */

__BOOL __queueCreate(u8 type, __PQUEUE queue, u32 item_qty, u32 item_size, u8* ptr, u32 ptr_size);
__BOOL __queueAdd(__PQUEUE queue, __PVOID data, u32 size);
__BOOL __queueGet(__PQUEUE queue, __PVOID data, u32* len);
/*! @brief To be implemented */
__BOOL __queuePeekFirst(__PQUEUE queue, __PVOID data, u32 size);
/*! @brief To be implemented */
__BOOL __queuePeekLast(__PQUEUE queue, __PVOID data, u32 size);
__BOOL __queueWaitForData(__PQUEUE queue, u32 timeout);
__BOOL __queueWaitForEmpty(__PQUEUE queue, u32 timeout);
__BOOL __queueIsReady(__PQUEUE queue);

/** @defgroup Queue_PublicMacros Public macros
  * @{
  */

/*!
 * @brief Return the quantity of items in the queue.
 */
#define __queueGetItemCount(x)		(x->item_count)

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_QUEUE */

#endif /* __QUEUE_H__ */
