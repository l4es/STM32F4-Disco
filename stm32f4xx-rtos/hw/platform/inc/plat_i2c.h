/***************************************************************************
 * plat_i2c.h
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

#ifndef __PLAT_I2C_H__
#define __PLAT_I2C_H__

#include <core/inc/device.h>

#if __CONFIG_COMPILE_I2C

/** @addtogroup I2C
  * @{
  */

/** @defgroup I2C_Platform Platform-related
  * @{
  */

/** @defgroup I2C_Stm32 STM32
  * @{
  */

/** @defgroup I2C_Stm32_Constants Constants
  * @{
  */

/** @defgroup I2C_Stm32_Modes Mode
  * @{
  */

#define __PLATI2C_MODE_INTERRUPTS	1	/*!< @brief If 1, the driver will work with interrupts,
													otherwise polling mode will be used. */

/**
  * @}
  */


/** @defgroup I2C_Stm32_DefaultDefines Default values
  *
  * If the parameter \c params if left to __NULL When calling __deviceInit()
  * to initialize the I2C driver, the driver will take these values as defaults.
  *
  * @{
  */

#define __PLATI2C_RXBUFF_LEN	64		/*!< @brief RX buffer length */
#define __PLATI2C_TXBUFF_LEN	64		/*!< @brief TX buffer length */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup I2C_Stm32_Typedefs Typedefs
  * @{
  */

/*!
 * @brief The I2C_PARAMS structure contains the required hardware information
 * to manage the I2C driver. Usually its values are pre-configured in a file
 * inside the \c boards directory.
 */
typedef struct {
	u32				scl_pin;			/*!< @brief SCL pin bit */
	u32				sda_pin;			/*!< @brief SDA pin bit */
	GPIO_TypeDef*	port_addr;			/*!< @brief Port address */
	I2C_TypeDef*	base_addr;			/*!< @brief I2C registers address */
	u32				bus_clock_addr;		/*!< @brief I2C bus clock address */
	u8				apb_bus_num;		/*!< @brief I2C APB bus number */
	u32				default_speed;		/*!< @brief I2C default speed */
	u8				rxstep;				/*!< @brief Used to manage RX steps */
	u8				subaddr_cnt;		/*!< @brief Counter of the sub-address bits sent */
} I2C_PARAMS, *PI2C_PARAMS;

/**
  * @}
  */

i32 __i2cPlatIoCtl(__PDEVICE dv, u32 code, u32 param, __PVOID in, u32 in_len, __PVOID out, u32 out_len);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_I2C */

#endif /* __PLAT_I2C_H__ */
