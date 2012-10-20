/***************************************************************************
 * i2c.h
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

#ifndef	__I2C_H__
#define	__I2C_H__

#include <core/inc/system.h>
#include <core/inc/intrvect.h>
#include <core/inc/device.h>
#include <plat_i2c.h>

#if __CONFIG_COMPILE_I2C

/** @addtogroup Driver
  * @{
  */

/** @addtogroup I2C
  * @{
  */

/** @defgroup I2C_Constants Constants
  * @{
  */

/** @defgroup I2C_MinimumBufferDefines Buffer
  * @{
  */

#define	__I2C_MINRXBUFLEN		__PLATI2C_RXBUFF_LEN		/*!< @brief Min RX buffer length */
#define	__I2C_MINTXBUFLEN		__PLATI2C_TXBUFF_LEN		/*!< @brief Min TX buffer length */

/**
  * @}
  */


/** @defgroup I2C_PlatIoCtlCodesDefines Platform IOCTLs
 *
 * The following values will be called from the I2C driver in order
 * to give the platform control over the I2C device hardware.
 * Users that may implement the I2C platform driver must create
 * a plat_i2c.h (and plat_i2c.c) file to manage the following
 * IO control codes with a @ref __DEV_PLAT_IOCTL type function
 * (\c dv_plat_ioctl member of the @ref __DEVICE structure). See
 * the __i2cPlatIoCtl() example function.
 *
 * The @ref __DEV_PLAT_IOCTL function should return an appropriate
 * error code like the following declared in @ref Device_ErrorCodes.
 *
 * @arg __DEV_OK 			Operation succeeded.
 * @arg __DEV_ERROR			Error detected.
 * @arg __DEV_WRITE_ERROR	Write error detected.
 * @arg __DEV_READ_ERROR 	Read error detected.
 * @arg __DEV_UNK_IOCTL 	The IO control code passed in the \c code argument
 * 							is unknown.
 * @arg __DEV_MEM_ERROR 	Memory error.
 * @arg __DEV_TIMEOUT 		Operation timeout.
 * @arg __DEV_BUSY			Device is busy.
 *
 * The return code for success is usually __DEV_OK unless another error code
 * is specified in the list below.
 *
 * @{
 */

/*!
 * @brief Called from __i2cOpen() to initialize the hardware.
 * The platform should initialize all the required hardware
 * to start the I2C.
 */
#define __I2C_PLAT_INIT_HW				1

/*!
 * @brief Deinitialize the hardware. Called from __i2cDestroy().
 */
#define __I2C_PLAT_DEINIT_HW			2

/*!
 * @brief Set the I2C speed. The speed value will be passed
 * in the \c param argument of the @ref __DEV_PLAT_IOCTL function.
 */
#define __I2C_PLAT_SET_SPEED			3

/*!
 * @brief Enable the device. Called for every __i2cFlush() and
 * __i2cRead() before starting the transmission/reception.
 */
#define __I2C_PLAT_ENABLE_DEVICE		4

/*!
 * @brief Disables the device. Called for every __i2cFlush() and
 * __i2cRead() after ending the transmission/reception.
 */
#define __I2C_PLAT_DISABLE_DEVICE		5

/*!
 * @brief Enables the interrupts of the I2C device. Called from
 * __i2cOpen().
 */
#define __I2C_PLAT_SET_IRQ				6

/*!
 * @brief Disables the interrupts of the I2C device. Called from
 * __i2cClose().
 */
#define __I2C_PLAT_RESET_IRQ			7

/*!
 * @brief Transmits the address of the remote device. The address
 * value will be  passed in the \c param argument of the
 * @ref __DEV_PLAT_IOCTL function.
 */
#define __I2C_PLAT_SET_ADDRESS			8

/*!
 * @brief Sets the address of the I2C device if it is working as a
 * slave (own address). The address value will be  passed in the
 * \c param argument of the @ref __DEV_PLAT_IOCTL function.
 */
#define __I2C_PLAT_SET_OWN_ADDRESS		9

/*!
 * @brief Transmits a START bit (START condition).
 */
#define __I2C_PLAT_GENERATE_START		10

/*!
 * @brief Transmits a STOP bit (STOP condition).
 */
#define __I2C_PLAT_GENERATE_STOP		11

/*!
 * @brief Checks for an event.
 *
 * Event codes are specified in the I2C @ref I2C_Events section. The
 * Event code will be passed in the \c param argument of the
 * @ref __DEV_PLAT_IOCTL function.
 */
#define __I2C_PLAT_CHECK_EVENT			12

/*!
 * @brief Configures automatic ACK generation.
 *
 * The binary value will be passed in the \c param argument of the
 * @ref __DEV_PLAT_IOCTL function. Activates the ACK generation when
 * \c param is equal to 1. Disables the ACK generation when \c param
 * is equal to 0.
 */
#define __I2C_PLAT_SET_ACK				13

/*!
 * @brief Aborts the current operation.
 *
 * The platform should reset its current state and abort any
 * transmission/reception, leaving the I2C device in a clean state.
 */
#define __I2C_PLAT_ABORT				14

/*!
 * @brief Queries the platform about the working mode.
 *
 * The I2C driver will call the @ref __DEV_PLAT_IOCTL function with this
 * IO control code to determine if the platform can process the requests
 * with interrupts. Otherwise the @ref I2C driver will try to work in
 * polling mode. The platform should return __DEV_OK when it is able to
 * manage the interrupts.
 */
#define __I2C_PLAT_USE_IRQ				15

/*!
 * @brief Used when the I2C driver works in interrupt mode. Instructs the
 * platform to start transmission. Called from __i2cFlush().
 */
#define __I2C_PLAT_INIT_TX				16

/*!
 * @brief Used when the I2C driver works in interrupt mode. Instructs the
 * platform to end the transmission. Called from __i2cFlush().
 */
#define __I2C_PLAT_END_TX				17

/*!
 * @brief Used when the I2C driver works in interrupt mode. Instructs the
 * platform to start reading from the device. Called from __i2cRead().
 */
#define __I2C_PLAT_INIT_RX				18

/*!
 * @brief Used when the I2C driver works in interrupt mode. Instructs the
 * platform to stop reading from the device. Called from __i2cRead().
 */
#define __I2C_PLAT_END_RX				19

/*!
 * @brief Write a single byte.
 */
#define __I2C_PLAT_WRITE_BYTE			20

/*!
 * @brief Read a single byte. The @ref __DEV_PLAT_IOCTL function should
 * return the read byte, or an error code (__DEV_ERROR for example) on
 * failure.
 */
#define __I2C_PLAT_READ_BYTE			21

/**
  * @}
  */

/** @defgroup I2C_Modes Modes
  * @{
  */

#define __I2C_MODE_SLAVE			0
#define __I2C_MODE_MASTER			1
#define __I2C_MODE_REPEAT_START		2

/**
  * @}
  */

/** @defgroup I2C_Events Events
  * @{
  */

#define __I2C_EVENT_MASTER_MODE			1
#define __I2C_EVENT_MASTER_TX			2
#define __I2C_EVENT_MASTER_RX			3
#define __I2C_EVENT_BYTE_TRANSMITTED	4
#define __I2C_EVENT_BYTE_RECEIVED		5


/**
  * @}
  */

#define	__I2C_ERR_OVERFLOW		0x01	/*!< @brief Receiver buffer overflow */

/**
  * @}
  */

/** @defgroup I2C_Typedefs Typedefs
  * @{
  */

/*!
 * @brief I2C driver Private Data block
 */

typedef struct	__i2cpdbTag {
	u32				pd_speed;		/*!< @brief Actual bus speed */
	u8				pd_busy;		/*!< @brief Bus busy flag */
	__VOLATILE u8	pd_mode;		/*!< @brief Master or slave / Repeat START */
	u32				pd_rxtmo;		/*!< @brief RX timeout */
	__VOLATILE u8	pd_rxerr;		/*!< @brief Receiver error */
	__VOLATILE u16	pd_rxqty;		/*!< @brief Quantity to read */
	__VOLATILE u16	pd_rcidx;		/*!< @brief Receiver buffer index */
	__VOLATILE u16	pd_rbidx;		/*!< @brief Read buffer index */
	__VOLATILE u16	pd_rxcnt;		/*!< @brief Received chars count */
	__VOLATILE u16	pd_rxlen;		/*!< @brief RX buffer length */
	pu8				pd_rxbuf;		/*!< @brief RX buffer */
	u32				pd_txtmo;		/*!< @brief TX timeout */
	__VOLATILE u16	pd_tcidx;		/*!< @brief Transmit buffer index */
	__VOLATILE u16	pd_tbidx;		/*!< @brief Write buffer index */
	__VOLATILE u16	pd_txcnt;		/*!< @brief Transmit chars count */
	__VOLATILE u16	pd_txlen;		/*!< @brief TX buffer length */
	pu8				pd_txbuf;		/*!< @brief TX buffer */
	u8				pd_rmtaddr;		/*!< @brief Remote address */
	u8				pd_ownaddr;		/*!< @brief Own address */
	u32				pd_subaddr;		/*!< @brief Sub address when reading */
	u8				pd_subaddrlen;	/*!< @brief Sub address length (8/16/32) */

} __I2C_PDB, *__PI2C_PDB;

/*!
 * @brief I2C driver custom initialization structure.
 * Pass this structure to initialize the driver (__i2cInit() / __deviceInit() function)
 * with custom parameters.
 */
typedef struct __i2cConfig
{
	u16		rxsize;		/*!< @brief RX buffer length */
	u16		txsize;		/*!< @brief TX buffer length */
} __I2C_CONFIG, *__PI2C_CONFIG;

/**
  * @}
  */

i32 __i2cInit(__PDEVICE dv, __PVOID params);
i32 __i2cDeinit(__PDEVICE dv);
i32 __i2cIOCtl(__PDEVICE dv, u32 cmd, u32 param, __PVOID data, u32 len);
i32 __i2cOpen(__PDEVICE dv, u32 mode);
i32 __i2cClose(__PDEVICE dv);
i32 __i2cSize(__PDEVICE dv, u8 mode);
i32 __i2cFlush(__PDEVICE dv);
i32 __i2cRead(__PDEVICE dv, __PVOID buf, u16 qty);
i32 __i2cWrite(__PDEVICE dv, __CONST __PVOID buf, u16 qty);

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_I2C */

#endif /* __I2C_H__ */

