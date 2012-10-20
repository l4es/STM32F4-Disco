/***************************************************************************
 * plat_uart.h
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

#ifndef __PLAT_UART_H__
#define __PLAT_UART_H__

#include <core/inc/device.h>

#if __CONFIG_COMPILE_SERIAL

/** @addtogroup Serial
  * @{
  */

/** @defgroup Serial_Platform Platform-related
  * @{
  */

/** @defgroup Serial_Stm32	STM32
  * @{
  */

/** @defgroup Serial_Stm32_Constants Constants
  * @{
  */

/** @defgroup Serial_Stm32_DefaultDefines Default values
  *
  * If the parameter \c params if left to __NULL When calling __deviceInit()
  * to initialize the Serial driver, the driver will take these values as defaults.
  * @{
  */

#define __PLATUART_RXBUFF_LEN		32
#define __PLATUART_TXBUFF_LEN		32

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup Serial_Stm32_Typedefs Typedefs
  * @{
  */

/*!
 * @brief The UART_PARAMS structure contains the required hardware information
 * to manage the Serial driver. Usually its values are pre-configured in a file
 * inside the \c boards directory.
 */

typedef struct {
	u32				tx_pin;				/*!< @brief TX pin */
	u32				rx_pin;				/*!< @brief RX pin */
	GPIO_TypeDef*	port_addr;			/*!< @brief GPIO port address (i.e. GPIOA) */
	USART_TypeDef*	base_addr;			/*!< @brief UART address (i.e. USART1) */
	u32				bus_clock_addr;		/*!< @brief APB bus (i.e. RCC_APB1Periph_USART2) */
	u8				apb_bus_num;		/*!< @brief APB bus number (i.e. 1) */
	u32				remap_value;		/*!< @brief Remap value (i.e. GPIO_Remap_USART1).
													Can be zero (no remap) */
} UART_PARAMS, *PUART_PARAMS;

/**
  * @}
  */

i32 __serialPlatIoCtl(__PDEVICE dv, u32 code, u32 param, __PVOID in, u32 in_len, __PVOID out, u32 out_len);


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_SERIAL */

#endif /* __PLAT_UART_H__ */
