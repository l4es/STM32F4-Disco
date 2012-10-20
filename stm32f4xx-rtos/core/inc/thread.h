/***************************************************************************
 * thread.h
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

#ifndef	__THREAD_H__
#define	__THREAD_H__

#include <plat_cpu.h>

/** @addtogroup Thread
  * @{
  */

/** @defgroup Thread_Constants Constants
  * @{
  */

/** @defgroup Thread_CreationDefaultsDefines Default values
  * @{
  */

#define	__TH_MAXNAMELEN			8			/*!< @brief Max thread name length */
#define	__TH_MINSTACKSIZE		64			/*!< @brief Minimum stack size */
#define	__TH_MINTICKS			1			/*!< @brief Minimum ticks */
#define	__TH_DEFSLEEPTICKS		1			/*!< @brief Default sleep ticks */
#define	__TH_DEFWAITTICKS		1			/*!< @brief Default wait ticks */

/**
  * @}
  */

/** @defgroup Thread_StatusDefines Thread status
  * @{
  */

#define	__THSTS_READY			0			/*!< @brief Thread is waiting for it's turn to run */
#define	__THSTS_RUNNING			1			/*!< @brief Thread is running */
#define	__THSTS_SLEEPING		2			/*!< @brief Thread is sleeping */
#define	__THSTS_WAITING			3			/*!< @brief Thread is waiting for an event */
#define	__THSTS_DISABLED		4			/*!< @brief Thread is disabled */
#define	__THSTS_FIRSTTIME		5			/*!< @brief Thread first time */
#define	__THSTS_DELETE			6			/*!< @brief Thread marked for deletion */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup Thread_TypeDefs Typedefs
  * @{
  */

typedef struct __threadTag {

#if __CONFIG_COMPILE_STRING
	__STRING			th_name[__TH_MAXNAMELEN + 1];	/*!< @brief Thread name */
#endif /* __CONFIG_COMPILE_STRING */

	__VOLATILE u8		th_status;						/*!< @brief Thread status */
	u32					th_priority;					/*!< @brief Thread priority (0-255) */
	u32					th_ttl;							/*!< @brief Time to live */
	u32					th_load;						/*!< @brief Time to live reload value */
	__VOLATILE u32 		th_wait;						/*!< @brief Time to wait/sleep */
	u16					th_stksize;						/*!< @brief Stack size in bytes */
	__VOLATILE u32		th_sp;							/*!< @brief Stack pointer save area */
	pu8					th_stkptr;						/*!< @brief Stack pointer memory block*/
	__PVOID				th_param;						/*!< @brief Pointer to thread parameter */
	struct __threadTag*	th_qprev;
	struct __threadTag* th_qnext;
	struct __threadTag* th_evprev;
	struct __threadTag* th_evnext;
	struct __threadTag*	th_lstnext;						/*!< @brief Next thread in creation list*/
} __THREAD, *__PTHREAD;

/**
  * @}
  */

extern __VOLATILE __PTHREAD __threadCurrent;
#define __threadGetCurrent()	__threadCurrent
#define __threadSetCurrent(x)	__threadCurrent = x

extern __VOLATILE __PTHREAD __threadReady;
#define __threadGetReady()	__threadReady


/** @defgroup Thread_FunctionPrototypes Prototypes
  * @{
  */

typedef	__VOID (__THREADFUNC)(__VOID);

/**
  * @}
  */


__BOOL		__threadCheckName(__CONST __PSTRING name);
__BOOL		__threadFind(__PTHREAD th);
__PTHREAD	__threadFindName(__CONST __PSTRING name);
__PTHREAD	__threadCreate(__CONST __PSTRING name, __THREADFUNC *func, u8 prio, u16 stack, u32 ticks, __PVOID param);
__BOOL		__threadDestroy(__PTHREAD th);
__BOOL		__threadChangeTime(__PTHREAD th, u32 time);
__VOID		__threadSleep(u32 ticks);
__PVOID		__threadGetParameter(__VOID);
__VOID		__threadDisable(__PTHREAD th);
__VOID		__threadEnable(__PTHREAD th);
__VOID		__threadChange(__VOID);
__VOID 		__threadProcessTick(__VOID);
u16			__threadGetCount(__VOID);
__PTHREAD	__threadGetChain(__VOID);
__VOID 		__threadMaintenance(__VOID);
__VOID		__threadSuspend(__PTHREAD th, u8 newstate);
__VOID 		__threadAddToReadyList(__PTHREAD th);
__VOID		__threadRemoveFromSuspended(__PTHREAD th);
__VOID		__threadYield(__VOID);

/**
  * @}
  */


#endif //__THREAD_H__
