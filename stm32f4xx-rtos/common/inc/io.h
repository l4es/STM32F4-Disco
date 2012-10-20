/***************************************************************************
 * io.h
 * (C) 2011 Ivan Meleca
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

#ifndef __IO_H__
#define __IO_H__

#include <plat_cpu.h>

#if __CONFIG_COMPILE_IO || defined(__DOXYGEN__)

/*!
 * @addtogroup Common
 * @{
 */

/*!
 * @defgroup IO IO support
 *
 * This module implements a very basic support for IO related functions.
 * It relays on plat_cpu.c functions like __cpuIoPinConfig() and others, that
 * need to be implemented in the platform.
 *
 * Applications using these functions must me aware that these may not produce the
 * same effect for different boards/platforms. For example, setting the pin 10 on port 1
 * as an output, that may be used to turn on a device on one board, may have a different
 * use in another board (it could even break something).
 *
 * The AD part of this module has to be completed.
 *
 * @{
 */

/*!
 * @defgroup IO_Constants Constants
 * @{
 */

/*!
 * @defgroup IO_Types IO types
 * @{
 */

#define __IO_MODE_OUTPUT			0	/*!< @brief Output */
#define __IO_MODE_OUTPUT_PP			1	/*!< @brief Output */
#define __IO_MODE_OUTPUT_OD			2	/*!< @brief Output */
#define __IO_MODE_INPUT				3	/*!< @brief Input */
#define __IO_MODE_INPUT_PU			4	/*!< @brief Input with pull-up */
#define __IO_MODE_INPUT_PD			5	/*!< @brief Input with pull-down */
#define __IO_MODE_INPUT_FLOATING	6	/*!< @brief Floating input */

/**
  * @}
  */

/**
  * @}
  */

__BOOL __ioPinConfig(u32 port, u32 pin, u32 mode);
__BOOL __ioPinRead(u32 port, u32 pin);
__VOID __ioPinWrite(u32 port, u32 pin, __BOOL val);

__BOOL __ioADConfig(u32 port, u32 pin);
u32 __ioADRead(u32 port, u32 pin);

__BOOL __ioLedConfig(u32 led);
__BOOL __ioLedSet(u32 led, __BOOL val);

__BOOL __ioKeyConfig(u32 key);
__BOOL __ioKeyRead(u32 key);

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_IO */

#endif /* __IO_H__ */

