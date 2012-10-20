/***************************************************************************
 * heap.c
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

#include "heap.h"
#include <common/inc/common.h>
#include <core/inc/system.h>

/** @addtogroup Core
  * @{
  */

/** @defgroup Heap Heap
  * Heap management functions.
  * @{
  */

/** @defgroup Heap_PrivateConstants Private constants
  * @{
  */

#define	__HEAPFREEFLAG	0x80000000
#define	__MAXEXCEEDED	16

/**
  * @}
  */

/** @defgroup Heap_PrivateVariables Private variables
  * @{
  */

__STATIC pu32	__heapPool;			/*!< @brief Heap start pointer */
__STATIC u32	__heapAvail;		/*!< @brief Available memory */
__STATIC u32	__heapSize;			/*!< @brief Original heap size */

/**
  * @}
  */

/** @defgroup Heap_Functions Functions
  * @{
  */

/*!
 * @brief Initializes the heap manager.
 *
 * Call this function to initialize the heap manager. The platform implementation
 * must provide the base address and the size. See __systemStart() function and the
 * __CPU_HEAP_BASE and __CPU_HEAP_SIZE macros.
 * @param 	mem			Heap base address.
 * @param	size		Heap size.
 * @return	nothing.
 */
__VOID	__heapInit(__PVOID mem, u32 size)
{
	__STATIC u8 heap_init = 0;

	if (heap_init == 0)
	{
		__heapPool = (pu32) mem;
		__memSet(mem,0,size);
		size -= sizeof(u32);
		*__heapPool = size | __HEAPFREEFLAG;
		__heapAvail = __heapSize = size;
		heap_init = 1;
	}
}

/*!
 * @brief Returns the available remaining heap size.
 * @return The available heap size.
 */
u32	__heapAvailable(__VOID)
{
	if (__heapAvail < 8) return 0;
	return(__heapAvail - (u32) 8);
}

/*!
 * @brief Return a normalized value module of 4.
 *
 * Internal use. Avoid calls to this functions outside heap.c file.
 * @param	size	The required size
 * @return	The size in module of 4.
 */
__STATIC u16 __heapNormalizeSize(u16 size)
{
	if (size & 0x0003)
	{
		size += (4 - (size & 0x0003));
	}

	return(size + sizeof(u32));
}

/*!
 * @brief Find the best area to allocate the requested size.
 *
 * Internal use. Avoid calls to this function outside heap.c file.
 * @param	size		Required memory size. Must be normalized.
 * @param	rs			Pointer to the remaining free space.
 * @return 	Pointer to the memory block or __NULL if there is not
 * available free space.
 */
__STATIC __PVOID __heapFindBest(u16 size, pu32 rs)
{
	pu32	ptr;
	pu8		bak;
	pu32	lastptr = __NULL;
	u32		lastsize = 0xFFFFFFFF;
	u32		avail;
	u32		sum = 0;
	u8		dsp;

	/* Check for available free space */
	if ((u32) size > __heapAvailable()) return(__NULL);

	/* Walk heap searching for the best empty space according to required memory size */
	ptr = __heapPool;
	bak = (pu8) ptr;
	for (;;)
	{
		avail = *ptr;
		(avail & __HEAPFREEFLAG) ? (dsp = 1) : (dsp = 0);
		avail &= ~__HEAPFREEFLAG;
		if (dsp)
		{
			if (avail == (u32) size)
			{
				/* found it! */
				*rs = 0;
				return(ptr);
			}

			/* If free memory block is greater than size + __MAXEXCEEDED then allow storage */
			if (avail >= (u32) size && avail <= ((u32) size + __MAXEXCEEDED))
			{
				*rs = avail - (u32) size;
				return(ptr);
			}

			if (avail > size && (avail - (u32) size) < lastsize)
			{
				/* Store the last best fit area */
				lastsize = avail - (u32) size;
				lastptr  = ptr;
			}
		}

		bak += (avail + sizeof(u32));

		ptr = (pu32) bak;
		sum += avail;
		if (sum > (__heapSize - 32)) break;
	}
	if (lastptr == __NULL) return(__NULL);
	if (!(*lastptr & __HEAPFREEFLAG)) return(__NULL);

	avail = *lastptr & ~__HEAPFREEFLAG;
	/* remaining space of the block */
	*rs = avail - (u32) size;
	return(lastptr);
}

/*!
 * @brief Allocates memory.
 *
 * Call this function to claim a free memory block from the heap.
 * @param	size	Requested size.
 * @return	Pointer to a free memory block or __NULL if no heap memory is available.
 */
__PVOID	__heapAlloc(u16 size)
{
	pu32		ptr;
	u32			rest = 0;

	if ((size = __heapNormalizeSize(size)) == 0) return(__NULL);

	__systemDisableScheduler();

	if ((ptr = (pu32) __heapFindBest(size,&rest)) == __NULL)
	{
		__systemEnableScheduler();
		return(__NULL);
	}

	if (rest <= (u32) __MAXEXCEEDED)
	{
		size += rest;
		__heapAvail -= (u32) size;
	} else
	{
		*(ptr + ((size + sizeof(u32)) / sizeof(u32))) = (rest -  (u32) sizeof(u32)) | __HEAPFREEFLAG;
		__heapAvail -= (u32) (size + sizeof(u32));
	}

	*ptr = (u32) size;
	*(ptr + (u32) (size / sizeof(u32))) = (u32) __threadGetCurrent();

	__systemEnableScheduler();

	return(ptr + 1);
}
/*!
 * @brief Allocates memory and fills it with zeroes.
 *
 * Generates a call to __heapAlloc() function, and then uses the __memSet() function
 * to fill the requested memory block with zeroes.
 * @param	size	Requested memory size.
 * @return	Pointer to a free memory block or __NULL if no heap memory is available.
 */
__PVOID	__heapAllocZero(u16 size)
{
	pu8		ptr;

	if ((ptr = __heapAlloc(size)) != __NULL)
	{
		__memSet(ptr,0,size);
	}
	return(ptr);
}

/*!
 * @brief Frees a previously allocated memory block.
 *
 * Call this function to free a claimed memory space returned by the __heapAlloc() function.
 * Beware that this function will not check the validity of the pointer passed as parameter.
 * The pointer must be exactly the same returned from the __heapAlloc() function.
 * The misuse of this function could lead to general system instability.
 * @param ptr			Pointer to the block of memory to be freed.
 * @return Nothing.
 */
__VOID	__heapFree(__PVOID ptr)
{
	pu32		p = (pu32) ptr - 1;

	/* Minimal sanity check */
	if (!(*p & __HEAPFREEFLAG)) {
		if ((pu32) ptr > __heapPool && (pu32) ptr < (__heapPool + (__heapSize / sizeof(u32))))
		{
			__heapAvail += *p;
			*p |= __HEAPFREEFLAG;
		}
	}
}

/*!
 * \brief Defragment free memory spaces.
 *
 * Call this function to perform a "defrag" on freed memory. Ensures system stability.
 * Call this anytime (better when the system is idling), and possibly with interrupts
 * disabled.
 * @return	Nothing.
 */
__VOID	__heapDefrag(__VOID)
{
	u32		val;
	u32		rsum = 0;
	pu32	ptr = __heapPool;
	pu32	rst = __NULL;

	while (ptr < (__heapPool + (__heapSize / sizeof(u32))))
	{
		val = *ptr;
		if (val & __HEAPFREEFLAG)
		{
			rsum += (val & ~__HEAPFREEFLAG);
			if (rst == __NULL)
			{
				rst = ptr;
			} else
			{
				rsum += sizeof(u32);
				__heapAvail += sizeof(u32);
			}
		} else
		{
			if (rsum > 0L)
			{
				*rst = rsum | __HEAPFREEFLAG;
				rst = __NULL;
				rsum = 0;
			}
		}

		ptr += (((val & ~__HEAPFREEFLAG) + sizeof(u32)) / sizeof(u32));
	}
	if (rsum > 0L) *rst = rsum | __HEAPFREEFLAG;
}

/*!
 * @brief Walks the heap.
 *
 * This function will walk through the heap and will call the provided __HEAPCALLBACK()
 * function on every block of memory found (free or not).
 * @param 	func 	A pointer to a __HEAPCALLBACK() function.
 * @param	arg		Optional parameter to pass to the __HEAPCALLBACK() function.
 */
__VOID	__heapWalk(__HEAPCALLBACK *func, __PVOID arg)
{
	u32			sz = 0;
	u32			size;
	u8			free;
	__PTHREAD	th;
	pu32		ptr = __heapPool;

	if (func != __NULL)
	{
		while (sz < (__heapSize - 4))
		{
			size = *ptr;
			(size & __HEAPFREEFLAG) ? (free = __TRUE) : (free = __FALSE);
			size &= ~__HEAPFREEFLAG;
			th = (__PTHREAD) *(ptr + (size / sizeof(u32)));
			if ((*func)(ptr,size,free,th,arg) == __FALSE) return;
			sz += size + (sizeof(u32) * 2);
			ptr += ((size / sizeof(u32)) + 1);
		}
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

