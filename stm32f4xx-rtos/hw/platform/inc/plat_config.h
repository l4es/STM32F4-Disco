/***************************************************************************
 * plat_config.h
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

#ifndef __PLAT_CONFIG_H__
#define __PLAT_CONFIG_H__

/** @addtogroup Platform
  * @{
  */

/** @defgroup Platform_STM32 STM32
  * @{
  */

/** @defgroup Platform_STM32_Compiler Compiler-related
  * @{
  */

/**
  * @}
  */


/** @defgroup PlatformConfig Platform configuration
  * @{
  */

/*! @brief Board definition */
#define BOARD_STM32F4DISCOVERY

#if defined(BOARD_STM32F4DISCOVERY)

/*
 * STM32F4DISCOVERY Board
 */

#include "stm32f4discovery.h"

/*! @brief Quantity of interrupts */
#define __PLATCONFIG_MAX_IRQ	84	// TODO

#endif // defined(BOARD_STM32F4DISCOVERY)

/*! @brief Check for Context Switch IRQ reentrancy */
#define __PLATCONFIG_CHECK_CS_REENTRY		1

/*! @brief Platform implementation will provide the interrupt source */
#define __PLATCONFIG_GET_IRQ_SOURCE			1

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __PLAT_CONFIG_H__ */
