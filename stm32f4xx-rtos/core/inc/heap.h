/***************************************************************************
 * heap.h
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

#ifndef	__HEAP_H__
#define	__HEAP_H__

#include <plat_ostypes.h>
#include "thread.h"

/** @addtogroup Heap
  * @{
  */

/** @defgroup Heap_FunctionPrototypes Prototypes
  * @{
  */

typedef	__BOOL (__HEAPCALLBACK)(__PVOID, u32, __BOOL, __PTHREAD, __PVOID arg);

/**
  * @}
  */


__PVOID		__heapAlloc(u16 size);
__PVOID		__heapAllocZero(u16 size);
__VOID		__heapFree(__PVOID ptr);
u32			__heapAvailable(__VOID);
__VOID		__heapInit(__PVOID mem, u32 size);
__VOID		__heapDefrag(__VOID);
__VOID		__heapWalk(__HEAPCALLBACK *func, __PVOID arg);

/**
  * @}
  */

#endif	/*__HEAP_H__ */
