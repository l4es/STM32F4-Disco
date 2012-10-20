/***************************************************************************
 * plat_spi.h
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

#ifndef	__PLAT_SPI_H__
#define	__PLAT_SPI_H__

#include <core/inc/device.h>

#if __CONFIG_COMPILE_SPI

/** @addtogroup SPI
  * @{
  */

/** @defgroup SPI_Platform Platform-related
  * @{
  */

/** @defgroup SPI_Stm32	STM32
  * @{
  */

/** @defgroup SPI_Stm32_Constants Constants
  * @{
  */

/** @defgroup SPI_Stm32_DefaultDefines Default values
  *
  * If the parameter \c params if left to __NULL When calling __deviceInit()
  * to initialize the SD driver, the driver will take these values as defaults.
  * @{
  */

#define __PLATSPI_RXBUFLEN BOARD_SPI_RXBUFLEN		/*!< @brief Default RX buffer length */
#define __PLATSPI_TXBUFLEN BOARD_SPI_TXBUFLEN 		/*!< @brief Default TX buffer length */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup SPI_Stm32_Typedefs Typedefs
  * @{
  */

/*!
 * @brief The SPI_PARAMS structure contains the required hardware information
 * to manage the SPI driver. Usually its values are pre-configured in a file
 * inside the \c boards directory.
 */

typedef struct {
	SPI_TypeDef*	base_addr;
	GPIO_TypeDef*	port_miso;
	u32				pin_miso;
	GPIO_TypeDef*	port_mosi;
	u32				pin_mosi;
	GPIO_TypeDef*	port_clk;
	u32				pin_clk;
	GPIO_TypeDef*	port_cs;
	u32				pin_cs;
	u32				remap_value;
	u16				direction;
	u32				prescaler;
	u32				cpol;
	u32				cpha;
	u32				nss_mode;

} SPI_PARAMS, *PSPI_PARAMS;

/**
  * @}
  */

i32 __spiPlatIoCtl(__PDEVICE dv, u32 code, u32 param, __PVOID in, u32 in_len, __PVOID out, u32 out_len);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_SPI */

#endif /* __PLAT_SPI_H__ */
