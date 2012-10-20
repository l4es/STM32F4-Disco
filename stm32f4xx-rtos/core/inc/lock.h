/***************************************************************************
 * lock.h
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

#ifndef __LOCK_H__
#define __LOCK_H__

#include <plat_ostypes.h>
#include "thread.h"
#include "event.h"

/** @addtogroup Lock
  * @{
  */

/** @defgroup Lock_Constants Constants
  * @{
  */

/** @defgroup Lock_StatesDefines Lock states
  * @{
  */

#define __LOCK_LOCKED			1			/*!< @brief Locked. */
#define __LOCK_RELEASED			0			/*!< @brief Released.	*/

/**
  * @}
  */

/** @defgroup Lock_TypesDefines Lock types
  * @{
  */

#define __LOCK_TYPE_NORMAL		1			/*!< @brief Type Normal. */
#define __LOCK_TYPE_CPU			2			/*!< @brief Use CPU specific method. */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup Lock_TypeDefs Typedefs
  * @{
  */

/*!
* Lock structure
*/
typedef struct __lockTag  __LOCK, *__PLOCK;

//	Forward struct declaration
struct __threadTag;

struct __lockTag {

	__VOLATILE u8					state;
	u8								type;
	__PTHREAD						owner;
	__PTHREAD						parent;
	__EVENT							event;
	__VOLATILE __PLOCK				next;
};

typedef struct __lockListTag  __LOCK_LIST, *__PLOCK_LIST;	/*!< @brief Lock list structure. */

struct __lockListTag {
	__PLOCK 		lock;
	__PLOCK_LIST 	next;
};

/**
  * @}
  */

__PLOCK __lockCreate(__VOID);
__BOOL	__lockOwn(__PLOCK lock, u32 wait);
__BOOL	__lockRelease(__PLOCK lock);
__VOID	__lockDestroy(__PLOCK lock);
u32		__lockGetCount(__VOID);
__PLOCK_LIST __lockGetList(__VOID);

/**
  * @}
  */

#endif
