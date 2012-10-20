/***************************************************************************
 * plat_cpu.c
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

#include "plat_cpu.h"
#include <core/inc/thread.h>
#include <core/inc/system.h>
#include <common/inc/io.h>

extern __VOID __boardInitHW(__VOID);

/** @addtogroup Platform_STM32
  * @{
  */

#define NVIC_INT_CTRL   		0xE000ED04			/*!< @brief Interrupt control state register - See Cortex-M4 Devices Generic User Guide (DUI0553A_cortex_m4_dgug.pdf) @page 227 */
#define NVIC_PENDSVSET  		0x10000000			/*!< @brief Value to trigger PendSV exception - See Cortex-M4 Devices Generic User Guide (DUI0553A_cortex_m4_dgug.pdf) @page 230 */
#define NVIC_PENDSVCLEAR  		(1 << 27)			/*!< @brief Value to clear PendSV pending bit - See Cortex-M4 Devices Generic User Guide (DUI0553A_cortex_m4_dgug.pdf) @page 230 */
#define WD_BASE					0x40003000			/*!< @brief Address of Independent Watchdog - See RM0090- Reference manual @page 467, stm32f4xx.h, stm32fxx_iwdg.h */
#define WD_IWDG_KR				WD_BASE
#define WD_IWDG_PR				(WD_BASE + 0x04)
#define WD_IWDG_RLR				(WD_BASE + 0x08)
#define WD_IWDG_SR				(WD_BASE + 0x0C)
#define DBGMCU_CR				0xE0042004			/*!< @brief Address of Debug MCU - See RM0090- Reference manual@page 1296, stm32f4xx_dbgmcu.h */
#define DBGMCU_APB1_FZ			(DBGMCU_CR + 0x04)	/*!< @brief Address of Debug MCU - See RM0090- Reference manual@page 1296, stm32f4xx_dbgmcu.h */
#define DBGMCU_APB2_FZ			(DBGMCU_CR + 0x08)	/*!< @brief Address of Debug MCU - See RM0090- Reference manual@page 1296, stm32f4xx_dbgmcu.h */
#define SYSTICK_RELOAD			(SystemCoreClock / 1000)

/** @defgroup PlatformFunctions Functions
  * @{
  */

/*!
 * @brief Initializes the clock.
 *
 * Initializes the microprocessor clock if needed.
 * As with __cpuInitHardware() the developer may implement this
 * initialization elsewhere. Called from __systemInit().
 *
 * @return Nothing.
 */
__STATIC __VOID __cpuInitClock(__VOID)
{
	SystemInit();
}

/*!
 * @brief Sets the STM32 PendSV interrupt.
 *
 * Internal platform function.
 *
 * @return Nothing.
 */
__STATIC __VOID __cpuInitSVCPendingInterrupt(__VOID)
{
	__pcd_SetSvcPend();
}

/*!
 * @brief Prepares thread stack frame.
 *
 * This function has to create and reorder the stack pointer passed as argument
 * in a way that when the thread is about to be executed for the first time, at
 * the moment of branching to the first instruction of the thread and popping the
 * registers, the stack is perfectly aligned, and the entry point of the function
 * of the thread is in the right place on the stack. For example, when using the
 * STM32, this function reorders the stack having in mind the way the CortexM3
 * restores the registers (in a particular order) when returning from an ISR.
 * Called from __threadCreate().
 *
 * @param	stkptr	Stack pointer start address.
 * @param	func	Pointer to thread execution function.
 * @param	param	Optional parameter to retrieve with __threadGetParameter().
 * @return	Stack pointer start address.
 */
u32 __cpuMakeStackFrame(u32 stkptr, __PVOID *func, __PVOID param)
{
	pu32 stk;

	stk = (pu32) stkptr;
	*(--stk)	= (u32)0x01000000L; 	/* xPSR */
	*(--stk) 	= (u32)func; 			/* Thread functions */
	*(--stk) 	= (u32)0x00000014L; 	/* R14 */
	*(--stk) 	= (u32)0x00000012L; 	/* R12 */
	*(--stk)	= (u32)0x00000003L; 	/* R3 */
	*(--stk) 	= (u32)0x00000002L; 	/* R2 */
	*(--stk)	= (u32)0x00000001L; 	/* R1 */
	*(--stk) 	= (u32)0;				/* R0 */
	 
	*(--stk) = (u32)0x00000011L; 	/* R11 */
	*(--stk) = (u32)0x00000010L; 	/* R10 */
	*(--stk) = (u32)0x00000009L; 	/* R9 */
	*(--stk) = (u32)0x00000008L; 	/* R8 */
	*(--stk) = (u32)0x00000007L; 	/* R7 */
	*(--stk) = (u32)0x00000006L; 	/* R6 */
	*(--stk) = (u32)0x00000005L; 	/* R5 */
    *(--stk)  = (u32)0x00000004L;	/* R4 */
    return (u32) stk;
}

/*!
 * @brief Initializes interrupts
 *
 * Initializes interrupts, interrupts priorities, reallocates the
 * vector table if needed. Called from __systemInit().
 *
 * @return Nothing.
 */
__VOID __cpuInitInterrupts(__VOID)
{
#ifdef  VECT_TAB_RAM  
	/* Set the Vector Table base location at 0x20000000 */ 
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
	/* Set the Vector Table base location at 0x08000000 */ 
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif

	/* Init SVCPend Interrupt */
	__cpuInitSVCPendingInterrupt();	
}

/*!
 * @brief Starts the STM32 SYSTICK timer.
 *
 * Internal platform function. Configures the SYSTICK timer.
 *
 * @return Nothing.
 */
__VOID __cpuSetTimerSYSTICK(__VOID)
{
	SysTick_Config(SYSTICK_RELOAD);
}

/*!
 * @brief Initializes platform optional timers.
 *
 * Start hardware timers, mainly used by the applications. Called from __systemInit().
 *
 * @return Nothing.
 */
__VOID __cpuInitTimers(__VOID)
{
}

/*!
 * @brief STM32 SYSTICK interrupt handler.
 *
 * As required in @ref platisrimpl, this function call
 * the __systemProcessTick() function.
 *
 * @return Nothing.
 */
__VOID __cpuSysTickHandler(__VOID)
{
	__systemEnterISR();
	__systemProcessTick();
	__systemLeaveISR();
}


/*!
 * @brief Extra configuration before entering first context switch.
 *
 * Called after the creation of the System Thread but before the first context
 * switching, to insert custom code before enabling interrupts and start changing
 * threads. Called from __systemCreateThread().
 *
 * @return Nothing.
 */
__VOID __cpuCustomCreateSystemThread(__VOID)
{
	__systemStop();
	__pcd_ZeroPSP();
	__systemStart();
}

/*!
 * @brief Forces a context switch.
 *
 * Used on __threadSleep(), __threadYield() and __eventWait().
 * Sets (schedules) a call to the context-switching function.
 *
 * @return Nothing.
 */
__VOID	__cpuScheduleThreadChange(__VOID)
{
	pu32 intreg = (pu32) NVIC_INT_CTRL;
	*intreg |= NVIC_PENDSVSET;
}

/*!
 * @brief Disables a forced context switch.
 *
 * Disables the context-switch pending call if it was previously configured.
 *
 * @return Nothing.
 */
__VOID	__cpuClearPendingThreadChange(__VOID) {

	pu32 intreg = (pu32) NVIC_INT_CTRL;
	*intreg |= NVIC_PENDSVCLEAR;
}

/*!
 * @brief Checks for a previously pended call to the scheduler.
 *
 * @return Nothing.
 */
__BOOL	__cpuThreadChangeScheduled(__VOID)
{
	pu32 intreg = (pu32) NVIC_INT_CTRL;
	if (*intreg & NVIC_PENDSVSET) return __TRUE;
	return __FALSE;
}

#if __CONFIG_ENABLE_WATCHDOG
/*!
 * @brief Starts the watchdog, if available.
 *
 * Starts the watchdog if any. Called from __systemInit().
 *
 * @return Nothing.
 */
__VOID	__cpuStartWatchdog(__VOID)
{
	pu32	reg_kr;
	pu32	reg_pr;
	pu32	reg_rlr;
	pu32	reg_dbg;

	reg_kr 	= (pu32) WD_IWDG_KR;
	reg_pr 	= (pu32) WD_IWDG_PR;
	reg_rlr = (pu32) WD_IWDG_RLR;
	reg_dbg = (pu32) DBGMCU_APB1_FZ;
	
	/* Set watchdog to stop in debug mode */
	*reg_dbg |= (1 << 12);	// TODO : Set DBG_IWDG_STOP
		
	/* Unprotect registers */
	*reg_kr = 0x5555;		// TODO : Enable access to IWDG_PR and IWDG_RLR
	
	/* Set prescaller and reload value */
	*reg_pr	= 0x00;			// TODO : Set Watchdog period
	*reg_rlr = 0xFFF;
	
	/* Start watchdog */
	*reg_kr = 0xCCCC;
}

/*!
 * @brief Resets the watchdog, if available.
 *
 * Resets the watchdog if any. Called from __systemInit().
 *
 * @return Nothing.
 */
__VOID	__cpuResetWatchdog(__VOID)
{
	pu32 reg_kr = (pu32) WD_IWDG_KR;
	*reg_kr = 0xAAAA;
}
#endif /* __CONFIG_ENABLE_WATCHDOG */

/*!
 * @brief Initializes the platform hardware.
 *
 * Main hardware initialization, like pins/ports direction.
 * This is optional because the developer may provide his own initialization.
 * Called from __systemInit().
 *
 * @return Nothing.
 */
__VOID	__cpuInitHardware(__VOID)
{
	/* Init Clock */
	__cpuInitClock();

	/* Call board-specific initialization */
	__boardInitHW();
}

/*!
 * @brief Initializes the main timer for context-switching.
 *
 * The OS needs just one timer to work. This timer will be the timer used in context
 * change operations, and determines the granularity of the time units of the system.
 * For example, when using the STM32, this function sets the SYSTICK timer to 1 millisecond.
 * Called from __systemInit().
 *
 * @return Nothing.
 */
__VOID 	__cpuInitSchedulerTimer(__VOID)
{
	__cpuSetTimerSYSTICK();
}

/*!
 * @brief Aligns the stack pointer address, if required.
 *
 * Used in the case the stack pointer of the thread has to be re-aligned.
 * Called from __threadCreate().
 *
 * @return Nothing.
 */
u32 __cpuStackFramePointer(pu8 stkptr, u32 stack)
{
	u32 ptr = 0;
	/* align address to 8-bit for stm32 */
	ptr = (u32) (stkptr + stack);
	ptr = ((u32) ptr & 0xFFFFFFFC);
	
	return ptr;
}

/*!
 * @brief Starts memory manager, if any.
 *
 * @return Nothing.
 */
__VOID	__cpuStartMMU(__VOID)
{
}

/*!
 * @brief Heartbeat, called each 100ms from the __systemThread().
 *
 * @return Nothing.
 */
__VOID	__cpuHeartBeat(__VOID)
{

#if defined(BOARD_HEARTBEAT_GPIO)
	if (BOARD_HEARTBEAT_GPIO->ODR & BOARD_HEARTBEAT_LED) {
		BOARD_HEARTBEAT_GPIO->ODR &= ~BOARD_HEARTBEAT_LED;
	} else {
		BOARD_HEARTBEAT_GPIO->ODR |= BOARD_HEARTBEAT_LED;
	}
#endif

}

__VOID __cpuDelayMs(u32 ms)
{
	u32 ticks = __systemGetTickCount();
	while (__systemGetTickCount() - ticks < ms);
}

#if __PLATCONFIG_GET_IRQ_SOURCE

/*!
 * @brief Called from __intArrival() to determine the interrupt number.
 *
 * Available if the platform has the __PLATCONFIG_GET_IRQ_SOURCE define set to 1.
 *
 * @param irq	Pointer to a 32 bit value, to store the irq number when the
 * 				function returns.
 *
 * @return		__TRUE if the irq number was found. Otherwise __FALSE.
 *
 */
__BOOL __cpuGetInterruptSource(u32* irq)
{
	i32 temp = ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) - 16);
	if (temp >= 0) {
		*irq = temp;
		return __TRUE;
	}

	return __FALSE;
}

#endif /* __PLATCONFIG_GET_IRQ_SOURCE */

/*!
 * @brief Configures a pin for the stm32 hardware, using the ST library.
 *
 * @return Nothing.
 */
__VOID __cpuPinConfigure(u32 pin, GPIO_TypeDef* port, GPIOSpeed_TypeDef speed, GPIOMode_TypeDef mode, GPIOOType_TypeDef outmode, GPIOPuPd_TypeDef pupd)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* TODO : Clock the port */
	if (port == GPIOA)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	} else if (port == GPIOB)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	} else if (port == GPIOC)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	} else if (port == GPIOD)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	} else if (port == GPIOE)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	} else if (port == GPIOF)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	} else if (port == GPIOG)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	} else if (port == GPIOH)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
	} else if (port == GPIOI)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
	}

	GPIO_InitStructure.GPIO_Mode 	= mode;
	GPIO_InitStructure.GPIO_Pin 	= pin;
	GPIO_InitStructure.GPIO_Speed 	= speed;
	GPIO_InitStructure.GPIO_OType 	= outmode;	//	TODO
	GPIO_InitStructure.GPIO_PuPd 	= pupd;		//	TODO

	GPIO_Init(port, &GPIO_InitStructure);
}

#if __CONFIG_COMPILE_IO

/*!
 * @brief Retrieves the pointer for a given IO port.
 * @param port Port number (1-based), being port 1 = GPIOA.
 * @return The pointer to the port registers. __NULL if not found.
 */
__STATIC GPIO_TypeDef* __cpuGetPort(u32 port)
{
	switch (port)
	{
		case 1:	return GPIOA;
		case 2:	return GPIOB;
		case 3:	return GPIOC;
		case 4:	return GPIOD;
		case 5:	return GPIOE;
		case 6:	return GPIOF;
		case 7:	return GPIOG;
		case 8:	return GPIOH;
		case 9:	return GPIOI;
	}

	return __NULL;
}

/*!
 * @brief Configures a pin according to the STM32.
 *
 * This function adds support to the @ref IO module.
 *
 * @param port	Port number (from 1 to 6). Pin 1 corresponds to GPIOA.
 * @param pin	Pin number (not a bitwise value).
 * @param mode	One of the @ref IO module @ref IO_Types.
 * @return __TRUE on success, otherwise __FALSE.
 *
 */
__BOOL __cpuIoPinConfig(u32 port, u32 pin, u32 mode)
{
	GPIO_TypeDef* p;
	GPIOSpeed_TypeDef speed = GPIO_Speed_50MHz;
	GPIOMode_TypeDef m;
	GPIOOType_TypeDef otype	= GPIO_OType_PP;
	GPIOPuPd_TypeDef pupd = GPIO_PuPd_NOPULL;


	if (pin > 31) return __FALSE;

	p = __cpuGetPort(port);
	if (!p) return __FALSE;

	switch (mode)
	{
		case __IO_MODE_OUTPUT:
		case __IO_MODE_OUTPUT_PP: m = GPIO_Mode_OUT; otype = GPIO_OType_PP; break;
		case __IO_MODE_OUTPUT_OD: m = GPIO_Mode_OUT; otype = GPIO_OType_OD; break;
		case __IO_MODE_INPUT_PU: m = GPIO_Mode_IN; pupd = GPIO_PuPd_UP; break;
		case __IO_MODE_INPUT_PD: m = GPIO_Mode_IN; pupd = GPIO_PuPd_DOWN; break;
		case __IO_MODE_INPUT:
		case __IO_MODE_INPUT_FLOATING:
			m = GPIO_Mode_IN; pupd = GPIO_PuPd_NOPULL; break;
		default: return __FALSE;
	}

	__cpuPinConfigure(1 << pin, p, speed, m, otype, pupd);
	return __TRUE;
}

/*!
 * @brief Reads the value of a pin.
 *
 * This function adds support to the @ref IO module.
 *
 * @param port	Port number (from 1 to 6). Pin 1 corresponds to GPIOA.
 * @param pin	Pin number (not a bitwise value).
 * @return __TRUE if the pin is high, __FALSE if the pin is low.
 *
 */
__BOOL __cpuIoPinRead(u32 port, u32 pin)
{
	GPIO_TypeDef* p = __cpuGetPort(port);
	if (pin > 31) return __FALSE;
	if (!p) return __FALSE;
	return (p->IDR & (1 << pin)) ? __TRUE : __FALSE;
}

/*!
 * @brief Writes a logical 1 or 0 to a pin.
 *
 * This function adds support to the @ref IO module.
 *
 * @param port	Port number (from 1 to 6). Pin 1 corresponds to GPIOA.
 * @param pin	Pin number (not a bitwise value).
 * @param value	__TRUE for a logical 1, __FALSE for a logical 0.
 * @return Nothing.
 *
 */
__VOID __cpuIoPinWrite(u32 port, u32 pin, __BOOL value)
{
	GPIO_TypeDef* p = __cpuGetPort(port);
	if (pin > 31) return;
	if (!p) return;

	if (value)
	{
		p->ODR |= (1 << pin);
	} else {
		p->ODR &= ~(1 << pin);
	}
}

/*!
 * @brief Configures an analogical input
 *
 * This function adds support to the @ref IO module.
 *
 * @note TODO
 *
 * @param port	Port number (from 1 to 6). Pin 1 corresponds to GPIOA.
 * @param pin	Pin number (not a bitwise value).
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __cpuIoADConfig(u32 port, u32 pin)
{
	/*
	 * TODO
	 */
	return __FALSE;
}

/*!
 * @brief Reads the current value of an analogical input.
 *
 * This function adds support to the @ref IO module.
 *
 * @note TODO
 *
 * @param port	Port number (from 1 to 6). Pin 1 corresponds to GPIOA.
 * @param pin	Pin number (not a bitwise value).
 * @return The current value of the analogical input.
 */
u32 __cpuIoADRead(u32 port, u32 pin)
{
	/*
	 * TODO
	 */
	return 0;
}

/*!
 * @brief Configures a led.
 *
 * The led must be supported by the board implementation.
 *
 * This function adds support to the @ref IO module.
 *
 * @param led	Led number.
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __cpuIoLedConfig(u32 led)
{
#ifdef BOARD_LED_COUNT

	GPIO_TypeDef* port;
	u32 pin;

	if (led == 0 || led > BOARD_LED_COUNT) return __FALSE;

	BOARD_GET_LED_PORT(led, port);
	BOARD_GET_LED_PIN(led, pin);

	if (!port) return __FALSE;

	__cpuPinConfigure(pin, port, GPIO_Speed_50MHz, GPIO_Mode_OUT, GPIO_OType_OD, GPIO_PuPd_NOPULL);
	return __TRUE;

#else

	return __FALSE;

#endif /* BOARD_LED_COUNT */
}

/*!
 * @brief Sets a logical value to a led.
 *
 * The led must be supported by the board implementation.
 *
 * This function adds support to the @ref IO module.
 *
 * @param led	Led number.
 * @param val	Logical value. __TRUE for logical 1, __FALSE for logical 0.
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __cpuIoLedSet(u32 led, __BOOL val)
{
#ifdef BOARD_LED_COUNT

	GPIO_TypeDef* port;
	u32 pin;

	if (led == 0 || led > BOARD_LED_COUNT) return __FALSE;

	BOARD_GET_LED_PORT(led, port);
	BOARD_GET_LED_PIN(led, pin);

	if (!port) return __FALSE;

	if (val)
	{
		port->ODR |= pin;
	} else {
		port->ODR &= ~pin;
	}
	return __TRUE;

#else

	return __FALSE;

#endif /* BOARD_LED_COUNT */
}

/*!
 * @brief Configures a key.
 *
 * The key must be supported by the board implementation.
 *
 * This function adds support to the @ref IO module.
 *
 * @param key	Key number.
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __cpuIoKeyConfig(u32 key)
{
#ifdef BOARD_KEY_COUNT

	GPIO_TypeDef* port;
	u32 pin;

	if (key == 0 || key > BOARD_LED_COUNT) return __FALSE;

	BOARD_GET_KEY_PORT(key, port);
	BOARD_GET_KEY_PIN(key, pin);

	if (!port) return __FALSE;

	__cpuPinConfigure(pin, port, GPIO_Speed_50MHz, GPIO_Mode_IN, GPIO_OType_PP, GPIO_PuPd_UP);
	return __TRUE;

#else

	return __FALSE;

#endif /* BOARD_KEY_COUNT */
}

/*!
 * @brief Sets a logical value to a led.
 *
 * The led must be supported by the board implementation.
 *
 * This function adds support to the @ref IO module.
 *
 * @param key	Led number.
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __cpuIoKeyRead(u32 key)
{
#ifdef BOARD_KEY_COUNT

	GPIO_TypeDef* port;
	u32 pin;

	if (key == 0 || key > BOARD_LED_COUNT) return __FALSE;

	BOARD_GET_KEY_PORT(key, port);
	BOARD_GET_KEY_PIN(key, pin);

	if (!port) return __FALSE;

	return (port->IDR & pin) ? __TRUE : __FALSE;

#else

	return __FALSE;

#endif /* BOARD_HAS_LEDS */

}


#endif /* __CONFIG_COMPILE_IO */

/**
  * @}
  */

/**
  * @}
  */

