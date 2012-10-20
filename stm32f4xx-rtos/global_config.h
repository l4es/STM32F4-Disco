/***************************************************************************
 * global_config.h
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

#ifndef __GLOBAL_CONFIG_H__
#define __GLOBAL_CONFIG_H__

/* Include here your application configuration header.
 * An application can override any of these values by defining
 * them in a "private" file.
 */
/* #include <apps/dummy/dummy_config.h> */

/*! @brief Debug terminal input device */
#if !defined(__CONFIG_DBGTERM_IN_DEVICE) || defined(__DOXYGEN__)
#define __CONFIG_DBGTERM_IN_DEVICE "serial1"

/*! @brief File to include for the output device */
#define __CONFIG_DBGTERM_IN_DEVICE_INC	<drivers/inc/serial.h>

/*! @brief Parameter to pass to the __deviceInit() function */
#define __CONFIG_DBGTERM_IN_DEVICE_INIT_MODE	(__DEV_WR_CRLF)

/*! @brief Parameter to pass to the __deviceOpen() function */
#define __CONFIG_DBGTERM_IN_DEVICE_OPEN_MODE	__CONFIG_DBGTERM_SPEED
#endif

/*! @brief Debug terminal output device */
#if !defined(__CONFIG_DBGTERM_OUT_DEVICE) || defined(__DOXYGEN__)
#define __CONFIG_DBGTERM_OUT_DEVICE	"serial1"

/*! @brief File to include for the output device */
#define __CONFIG_DBGTERM_OUT_DEVICE_INC	<drivers/inc/serial.h>

/*! @brief Parameter to pass to the __deviceInit() function */
#define __CONFIG_DBGTERM_OUT_DEVICE_INIT_MODE	(__DEV_WR_CRLF)

/*! @brief Parameter to pass to the __deviceOpen() function */
#define __CONFIG_DBGTERM_OUT_DEVICE_OPEN_MODE	__CONFIG_DBGTERM_SPEED
#endif

/*! @brief Debug terminal line length */
#if !defined(__CONFIG_DBGTERM_LINE_LENGTH) || defined(__DOXYGEN__)
#define __CONFIG_DBGTERM_LINE_LENGTH	64
#endif

/*! @brief Debug terminal speed rate, if using serial */
#if !defined(__CONFIG_DBGTERM_SPEED) || defined(__DOXYGEN__)
#define __CONFIG_DBGTERM_SPEED			115200
#endif

/*! @brief Call heap defrag periodically */
#if !defined(__CONFIG_CALL_HEAP_DEFRAG) || defined(__DOXYGEN__)
#define __CONFIG_CALL_HEAP_DEFRAG		0
#endif

/*! @brief Time in milliseconds to call heap defrag */
#if !defined(__CONFIG_HEAP_DEFRAG_TIME) || defined(__DOXYGEN__)
#define __CONFIG_HEAP_DEFRAG_TIME		30000
#endif

/*! @brief System thread priority */
#if !defined (__CONFIG_PRIO_SYSTHREAD) || defined(__DOXYGEN__)
#define __CONFIG_PRIO_SYSTHREAD			50
#endif

/*! @brief System thread stack */
#if !defined (__CONFIG_STACK_SYSTHREAD) || defined(__DOXYGEN__)
#define __CONFIG_STACK_SYSTHREAD		512
#endif

/*! @brief Terminal thread priority */
#if !defined(__CONFIG_PRIO_TERMTHREAD) || defined(__DOXYGEN__)
#define __CONFIG_PRIO_TERMTHREAD		100
#endif

/*! @brief Enable a thread for software timers by default */
#if !defined(__CONFIG_ENABLE_SWTIMERS) || defined(__DOXYGEN__)

#define __CONFIG_ENABLE_SWTIMERS		0

/*! @brief Timer thread priority */
#if !defined(__CONFIG_PRIO_TIMTHREAD) || defined(__DOXYGEN__)
#define __CONFIG_PRIO_TIMTHREAD			100
#endif

/*! @brief Timer thread stack size */
#if !defined(__CONFIG_STACK_TIMTHREAD) || defined(__DOXYGEN__)
#define __CONFIG_STACK_TIMTHREAD		256
#endif

#endif

/*! @brief The time in milliseconds the system thread should sleep */
#if !defined(__CONFIG_SYSTHREAD_SLEEP_TIME) || defined (__DOXYGEN__)
#define __CONFIG_SYSTHREAD_SLEEP_TIME 	100
#endif

/*! @brief Terminal commands to keep in historic */
#if !defined(__CONFIG_TERM_HIST_DEPTH) || defined(__DOXYGEN__)
#define __CONFIG_TERM_HIST_DEPTH		3
#endif

/*! @brief OS IO support module */
#if !defined(__CONFIG_COMPILE_IO) || defined(__DOXYGEN__)
#define __CONFIG_COMPILE_IO				1
#endif

/*! @brief OS uses strings module */
#if !defined(__CONFIG_COMPILE_STRING) || defined(__DOXYGEN__)
#define __CONFIG_COMPILE_STRING			1
#endif

/*! @brief Compile terminal module */
#if !defined(__CONFIG_COMPILE_TERMINAL) || defined(__DOXYGEN__)
#define __CONFIG_COMPILE_TERMINAL		1
#endif

/*! @brief Compile the Debug Terminal commands. */
#if !defined(__CONFIG_COMPILE_DBGTERM) || defined(__DOXYGEN__)
#define __CONFIG_COMPILE_DBGTERM		1
#endif

/*! @brief Initialize a debug terminal from the system thread */
#if !defined(__CONFIG_DBGTERM_ENABLED) || defined(__DOXYGEN__)
#define __CONFIG_DBGTERM_ENABLED		1
#endif

#if (__CONFIG_DBGTERM_ENABLED && (!__CONFIG_COMPILE_TERMINAL || !__CONFIG_COMPILE_DBGTERM))
#error "Debug terminal needs __CONFIG_COMPILE_TERMINAL and __CONFIG_COMPILE_DBGTERM"
#endif

/*! @brief Enable watchdog */
#if !defined(__CONFIG_ENABLE_WATCHDOG) || defined(__DOXYGEN__)
#define __CONFIG_ENABLE_WATCHDOG		1
#endif

/*! @brief Enable heart beat */
#if !defined(__CONFIG_ENABLE_HEARTBEAT) || defined(__DOXYGEN__)
#define __CONFIG_ENABLE_HEARTBEAT		1
#endif

/*! @brief Compile serial driver */
#if !defined(__CONFIG_COMPILE_SERIAL) || defined(__DOXYGEN__)
#define __CONFIG_COMPILE_SERIAL			1
#endif

/*! @brief Compile spi driver */
#if !defined(__CONFIG_COMPILE_SPI) || defined(__DOXYGEN__)
#define __CONFIG_COMPILE_SPI			1
#endif

/*! @brief Compile Queue module */
#if !defined(__CONFIG_COMPILE_QUEUE) || defined(__DOXYGEN__)
#define __CONFIG_COMPILE_QUEUE			1
#endif

/*! @brief Compile I2C Driver */
#if !defined(__CONFIG_COMPILE_I2C) || defined(__DOXYGEN__)
#define __CONFIG_COMPILE_I2C			1
#endif

/*! @brief RTC support */
#if !defined(__CONFIG_COMPILE_RTC) || defined(__DOXYGEN__)
#define __CONFIG_COMPILE_RTC			1
#endif

/*! @brief Post stack overflow enabled */
#if !defined(__CONFIG_POST_STACK_OVERFLOW) || defined(__DOXYGEN__)
#define __CONFIG_POST_STACK_OVERFLOW	0
#endif

/*! @brief If set to any value but zero, it will force the priority
 * value of all the threads with the value defined here below. */
#if !defined(__CONFIG_DEBUG_RR) || defined(__DOXYGEN__)
#define __CONFIG_DEBUG_RR				0
#endif

#endif /* __GLOBAL_CONFIG_H__ */
