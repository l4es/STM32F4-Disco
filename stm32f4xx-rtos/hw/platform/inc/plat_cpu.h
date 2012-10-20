/***************************************************************************
 * plat_cpu.h
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

#ifndef __PLAT_CPU_H__
#define	__PLAT_CPU_H__

#if defined  (HSE_VALUE)
/* Redefine the HSE value; it's equal to 8 MHz on the STM32F4-DISCOVERY Kit */
 #undef HSE_VALUE
 #define HSE_VALUE    ((uint32_t)8000000)
#endif /* HSE_VALUE */

#include "global_config.h"
#include "plat_config.h"
#include "plat_ostypes.h"
#include "plat_comp_dep.h"
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_flash.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_spi.h>
#include <stm32f4xx_usart.h>
#include <misc.h>

#define __pinGet(x, y) ((x)->ODR & y)
#define __pinSet(x, y, z) (__VOID)((z) ? ((x)->ODR |= (y)) : ((x)->ODR &= ~(y)))
#define __portSet(x, y) ((x)->ODR = y)

#define CPU_TICKS_TO_MS(x)			((u32)(x))
#define CPU_MS_TO_TICKS(x)			((u32)(x))

/** @addtogroup Platform_STM32
  * @{
  */

/** @addtogroup PlatformFunctions Functions
  * @{
  */

/*!
 * @brief Disable interrupts.
 *
 * @return Nothing.
 */
#define __cpuDisableInterrupts()		__pcd_DisableIRQs()

/*!
 * @brief Enable interrupts.
 *
 * @return Nothing.
 */
#define __cpuEnableInterrupts()			__pcd_EnableIRQs()

/*!
 * @brief OS in entering IDLE mode.
 *
 * @return Nothing.
 */
#define	__cpuIdleIn()


/*!
 * @brief OS in exiting IDLE mode.
 *
 * @return Nothing.
 */
#define	__cpuIdleOut()

/**
  * @}
  */

/**
  * @}
  */

/*
 * The following macros add support for the NET module.
 */
#define __byte_swap2(val)           \
    (((val & 0xff) << 8) |          \
     ((val & 0xff00) >> 8))
#define __byte_swap4(val)           \
    (((val & 0xff) << 24) |         \
     ((val & 0xff00) << 8) |        \
     ((val & 0xff0000) >> 8) |      \
     ((val & 0xff000000) >> 24))
#define	__htons(x)	__byte_swap2(x)
#define	__ntohs(x)	__byte_swap2(x)
#define	__htonl(x)	__byte_swap4(x)
#define	__ntohl(x)	__byte_swap4(x)
#define __chksum_16bit_value(ptr) (u16) ( ((u16) *ptr << 8) | *(ptr + 1) )
#define __chksum_8bit_value(ptr) (u16) ( ((u16) *ptr << 8) )

/*
 * Mandatory.
 */
__VOID __cpuInitHardware(__VOID);
__VOID __cpuInitTimers(__VOID);
__VOID __cpuInitSchedulerTimer(__VOID);
u32 __cpuMakeStackFrame(u32 stkptr, __PVOID *func, __PVOID param);
u32 __cpuStackFramePointer(pu8 stkptr, u32 stack);
__VOID __cpuInitInterrupts(__VOID);
__VOID __cpuCustomCreateSystemThread(__VOID);
__VOID __cpuScheduleThreadChange(__VOID);
__VOID __cpuClearPendingThreadChange(__VOID);
__BOOL __cpuThreadChangeScheduled(__VOID);
__VOID __cpuStartMMU(__VOID);
__VOID __cpuStartWatchdog(__VOID);
__VOID __cpuResetWatchdog(__VOID);
__VOID __cpuDelayMs(u32 ms);

/*
 * Optional.
 */
__VOID __cpuHeartBeat(__VOID);
__VOID __cpuPinConfigure(u32 pin, GPIO_TypeDef* port, GPIOSpeed_TypeDef speed, GPIOMode_TypeDef mode, GPIOOType_TypeDef outmode, GPIOPuPd_TypeDef pupd);
__BOOL __cpuGetInterruptSource(u32* irq);

/*
 * Basic support for the IO module.
 */
#if __CONFIG_COMPILE_IO
__BOOL __cpuIoPinConfig(u32 port, u32 pin, u32 mode);
__BOOL __cpuIoPinRead(u32 port, u32 pin);
__VOID __cpuIoPinWrite(u32 port, u32 pin, __BOOL value);
__BOOL __cpuIoADConfig(u32 port, u32 pin);
u32 __cpuIoADRead(u32 port, u32 pin);
__BOOL __cpuIoLedConfig(u32 led);
__BOOL __cpuIoLedSet(u32 led, __BOOL val);
__BOOL __cpuIoKeyConfig(u32 key);
__BOOL __cpuIoKeyRead(u32 key);
#endif /* __CONFIG_COMPILE_IO */

#endif /*__PLAT_CPU_H__ */
