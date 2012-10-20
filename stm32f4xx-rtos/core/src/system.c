/***************************************************************************
 * system.c
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

#include "system.h"
#include "dbgterm.h"
#include "heap.h"
#include "thread.h"
#include "timer.h"
#include "device.h"
#include "rtc.h"

/** @defgroup Milos Milos
  * @{
  */

/** @defgroup Driver Device drivers
  * @{
  */

/**
  * @}
  */

/** @defgroup Network Network
  * @{
  */

/**
  * @}
  */

/** @defgroup Platform Platform
  * Platform-dependent code.
  * @{
  */

/**
  * @}
  */

/** @defgroup Core Core
  * @{
  */

/** @defgroup System System
  * @brief System functions.
  * @{
  */

/** @defgroup System_PrivateVariables Private variables
  * @{
  */

__BOOL 	__systemReset = __FALSE;						/*!< @brief Global flag, stops watchdog */
__STATIC __APP_ENTRY* 	__systemAppEntry = __NULL;		/*!< @brief Application entry point */
__STATIC __VOLATILE u32	__systemIrqCount;				/*!< @brief Keeps track of disabled IRQ */
__STATIC __VOLATILE u32	__systemTickCount = 0;			/*!< @brief System ticks counter */
__STATIC __VOLATILE u32	__systemSecondsCount = 0;		/*!< @brief System seconds counter */
__STATIC __VOLATILE u32	__systemContextSwCount = 0;		/*!< @brief Keeps track of disabled
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 context switching */
__STATIC __VOLATILE u32	__systemNestingISR = 0;			/*!< @brief Keeps track of ISR calls */
__STATIC __PTHREAD		__systemThreadPtr;				/*!< @brief Pointer to the system thread */
__STATIC __VOLATILE u8	__systemContextSwRequest;		/*!< @brief Context switch requested */

/**
  * @}
  */

/** @defgroup System_Functions Functions
  * @{
  */


/*!
 * @brief System thread.
 *
 * Internal thread, created after system initialization (__systemCreateThread() function).
 * Calls the application's entry point and \c should perform maintenance tasks.
 * @return Nothing.
 * @todo Add maintenance tasks.
 * @todo Add __heapDefrag() as user option.
 */
__VOID __systemThread(__VOID)
{
	u32 ticks = __systemTickCount;

	/* Start thread for software timers */
#if __CONFIG_ENABLE_SWTIMERS
	__timerInit(__CONFIG_STACK_TIMTHREAD);
#endif

	/* Initialize the Debug Terminal, if enabled */
#if __CONFIG_DBGTERM_ENABLED
	__dbgInit();
#endif // __CONFIG_DBGTERM_ENABLED

	if (__systemAppEntry)
	{
		(__systemAppEntry)();
	}
	
	for(;;)
	{

#if __CONFIG_ENABLE_WATCHDOG
		if (!__systemReset)
		{
//			__cpuResetWatchdog();
		}
#endif /* __CONFIG_ENABLE_WATCHDOG */
		
#if __CONFIG_ENABLE_HEARTBEAT
		__cpuHeartBeat();
#endif /* __CONFIG_ENABLE_HEARTBEAT */

		/* Seconds counter */
		if (__systemTickCount < ticks)
		{
			/* overflowed */
			ticks = 0xFFFFFFFF - ticks;
		}

		if (__systemTickCount - ticks > CPU_MS_TO_TICKS(1000))
		{
			__systemSecondsCount += (__systemTickCount - ticks) / CPU_MS_TO_TICKS(1000);
			ticks = __systemTickCount;
		}

		__threadSleep(__CONFIG_SYSTHREAD_SLEEP_TIME);
	}
}

/*!
 * @brief Creates the system threads.
 *
 * Internal use. Called from __systemInit() function.
 * @return Nothing.
 */
__STATIC __VOID __systemInitThread(__VOID)
{

	/* Create system thread */
	__systemThreadPtr = __threadCreate(	"system",						/* name 		*/
										__systemThread,					/* function 	*/
										__CONFIG_PRIO_SYSTHREAD,		/* priority 	*/
										__CONFIG_STACK_SYSTHREAD,		/* stack 		*/
										1,								/* time to live */
										__NULL);						/* parameter 	*/

	/* Call platform custom initialization before activating
	 * the scheduler
	 */
	__cpuCustomCreateSystemThread();
}

/*!
 * @brief System initialization.
 *
 * Call this function to start the OS. Provide an __APP_ENTRY() function pointer that will be
 * called from the System thread upon initialization. This function is intended to never return,
 * the application will take control of the system from the __APP_ENTRY() function.
 *
 * @param entry		Pointer to the application entry function.
 *
 * @return Nothing.
 */
__VOID __systemInit(__APP_ENTRY* entry)
{
	/* Avoid interrupts we are not handling yet */
	__systemStop();
	
	__systemAppEntry = entry;
	
	/* Init CPU hardware */
	__cpuInitHardware();
	
	/* MMU */
	__cpuStartMMU();

	/* Init Interrupts */
	__cpuInitInterrupts();

	/* Init Heap */
	__heapInit(&__CPU_HEAP_BASE, (u32) &__CPU_HEAP_SIZE);

	/* Init RTC */
#if __CONFIG_COMPILE_RTC
	__rtcInit();
#endif /* __CONFIG_COMPILE_RTC */

	/* Initialize Watchdog */
#if __CONFIG_ENABLE_WATCHDOG
//	__cpuStartWatchdog();
#endif /* __CONFIG_ENABLE_WATCHDOG */
	
	/* Create System Threads */
	__systemInitThread();

	/* Init Scheduler timer after creating the first thread */
	__cpuInitSchedulerTimer();

	__systemScheduleThreadChange();

	/* Enable interrupts */
	__systemStart();

	/* If everything is OK, we should be running the OS by now.
	 * This function will never return. __systemThread() will now enter execution under
	 * the context of the OS.
	 */
	for (;;)
	{
	}
}

/*!
 * @brief Manages the system tick timer.
 *
 * Called from platform implementation of the system tick timer.
 *
 * @return Nothing.
 */
__VOID __systemProcessTick(__VOID)
{
	/* Software system ticks */
	__systemTickCount++;
	__threadProcessTick();
}

/*!
 * @brief Disables interrupts.
 *
 * @return Nothing.
 */
__VOID __systemStop(__VOID)
{
	if (!__systemIrqCount)
	{
		__cpuDisableInterrupts();
	}
	__systemIrqCount++;
}

/*!
 * @brief Enables interrupts.
 *
 * @return Nothing.
 */
__VOID __systemStart(__VOID)
{
	if (__systemIrqCount && --__systemIrqCount) {
		return;
	}

	/* enable interrupts */
	__cpuEnableInterrupts();
}


/*!
 * @brief Enables scheduler.
 *
 * Enables context switching.
 * This function will check if a previous request from a context change was made, while the scheduler
 * was disabled. If so, and the first thread in the ready list has a higher priority than the current one,
 * a context switch will take place.
 *
 * @return Nothing.
 */
__VOID __systemEnableScheduler(__VOID)
{
	__systemStop();
	if (__systemContextSwCount)
	{
		__systemContextSwCount--;

		/* Check for pending thread change request */
		if (!__systemContextSwCount && __systemContextSwRequest)
		{
			__systemContextSwRequest = 0;

			/* Check if there is a ready thread with higher priority
			 * than the current one.
			 */
			if (__threadGetCurrent()) {
				if (__threadGetCurrent()->th_priority > __threadGetReady()->th_priority)
				{
					__threadAddToReadyList(__threadGetCurrent());
					__threadSetCurrent(__NULL);
					__systemScheduleThreadChange();
				}
			}
		}
	}
	__systemStart();
}

/*!
 * @brief Disables scheduler.
 *
 * Disables context switching.
 *
 * @return Nothing.
 */
__VOID __systemDisableScheduler(__VOID)
{
	__systemStop();
	__systemContextSwCount++;
	if (__cpuThreadChangeScheduled())
		__cpuClearPendingThreadChange();
	__systemStart();
}

/*!
 * @brief Schedules a context switch.
 *
 * @return Nothing.
 */
__VOID __systemScheduleThreadChange(__VOID)
{
	if (!__systemContextSwCount)
	{
		__cpuScheduleThreadChange();
	} else
	{
		__systemContextSwRequest = 1;
	}
}

/*!
 * @brief Enter ISR
 *
 * @return Nothing.
 */
__VOID __systemEnterISR(__VOID)
{
	__systemNestingISR++;
}

/*!
 * @brief Leave ISR
 *
 * @return Nothing.
 */

__VOID __systemLeaveISR(__VOID)
{
	__systemNestingISR--;
}

/*!
 * @brief Gets the system tick count from the last reset.
 *
 * @return The count of system ticks.
 */

u32 __systemGetTickCount(__VOID)
{
	return __systemTickCount;
}

/*!
 * @brief Gets the seconds passed from the last reset.
 *
 * @return The seconds passed from the last reset.
 */
u32 __systemGetSecondsCount(__VOID)
{
	return __systemSecondsCount;
}

/*!
 * @brief Gets the nested interrupt count.
 *
 * @return The nested interrupt count.
 */
u32 __systemGetIrqCount(__VOID)
{
	return __systemIrqCount;
}

/*!
 * @brief Call this function to check if the scheduler is disabled.
 *
 * @return __TRUE is the scheduler is disabled, otherwise __FALSE.
 */
__BOOL __systemSchedulerDisabled(__VOID)
{
	if (__systemContextSwCount) return __TRUE;
	return __FALSE;
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

/**
  * @}
  */


