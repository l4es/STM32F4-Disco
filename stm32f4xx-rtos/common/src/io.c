/***************************************************************************
 * io.c
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

#include "io.h"

#if __CONFIG_COMPILE_IO || defined(__DOXYGEN__)

/*!
 * @addtogroup IO
 * @{
 */

/*!
 * @defgroup IO_Functions Functions
 * @{
 */

/*!
 * @brief Configures a pin.
 *
 * @param port	A value (1-based) representing a port.
 * @param pin	Pin number (not a bitwise value).
 * @param mode	Mode, one of the following values:
 * 	@arg	__IO_MODE_OUTPUT			Output.
 * 	@arg	__IO_MODE_OUTPUT_PP			Push-pull output.
 * 	@arg	__IO_MODE_OUTPUT_OD			Open-drain output.
 * 	@arg	__IO_MODE_INPUT				Input.
 * 	@arg	__IO_MODE_INPUT_PU			Input with pull-up.
 * 	@arg	__IO_MODE_INPUT_PD			Input with pull-down.
 * 	@arg	__IO_MODE_INPUT_FLOATING	Floating input.
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __ioPinConfig(u32 port, u32 pin, u32 mode)
{
	return __cpuIoPinConfig(port, pin, mode);
}

/*!
 * @brief Reads the value of a pin.
 * @param port	A value (1-based) representing a port.
 * @param pin	Pin number (not a bitwise value).
 * @return __TRUE if the pin is high, __FALSE if the pin is low.
 */
__BOOL __ioPinRead(u32 port, u32 pin)
{
	return __cpuIoPinRead(port, pin);
}

/*!
 * @brief Writes a logical value to a pin.
 * @param port	A value (1-based) representing a port.
 * @param pin	Pin number (not a bitwise value).
 * @param val	Value, __TRUE for a logical 1, __FALSE for a logical 0.
 * @return Nothing.
 */
__VOID __ioPinWrite(u32 port, u32 pin, __BOOL val)
{
	__cpuIoPinWrite(port, pin, val);
}

/*!
 * @brief Configures a analogical input.
 * @param port	A value (1-based) representing a port.
 * @param pin	Pin number (not a bitwise value).
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __ioADConfig(u32 port, u32 pin)
{
	return __cpuIoADConfig(port, pin);
}

/*!
 * @brief Reads the current value of an analogical input.
 * @param port	A value (1-based) representing a port.
 * @param pin	Pin number (not a bitwise value).
 * @return The current value of the analogical input.
 */
u32 __ioADRead(u32 port, u32 pin)
{
	return __cpuIoADRead(port, pin);
}

/*!
 * @brief Configures a led.
 * @param led	A value (1-based) representing a led.
 * @return __TRUE on success, otherwise __FALSE;
 */
__BOOL __ioLedConfig(u32 led)
{
	return __cpuIoLedConfig(led);
}

/*!
 * @brief Turn on or off a led.
 * @param led	A value (1-based) representing a led.
 * @param val	On/Off state: __TRUE for ON, __FALSE for OFF.
 * @return __TRUE on success, otherwise __FALSE;
 */
__BOOL __ioLedSet(u32 led, __BOOL val)
{
	return __cpuIoLedSet(led, val);
}

/*!
 * @brief Configures a key.
 * @param key	A value (1-based) representing a key.
 * @return __TRUE on success, otherwise __FALSE;
 */
__BOOL __ioKeyConfig(u32 key)
{
	return __cpuIoKeyConfig(key);
}

/*!
 * @brief Reads the current value of a key..
 * @param key	A value (1-based) representing a key.
 * @return The current value of the key.
 */
__BOOL __ioKeyRead(u32 key)
{
	return __cpuIoKeyRead(key);
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_IO */
