/***************************************************************************
 * spi.h
 * (C) 2010 Ivan Meleca
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

#ifndef	__SPI_H__
#define	__SPI_H__

#include <core/inc/system.h>
#include <core/inc/intrvect.h>
#include <core/inc/device.h>
#include <plat_spi.h>

/** @addtogroup Driver
  * @{
  */

/** @addtogroup SPI
  * @{
  */

/** @defgroup SPI_Constants Constants
  * @{
  */

/** @defgroup SPI_MinimumBufferDefines Buffer
  * @{
  */

#define	__SPI_MINRXBUFLEN		32		/*!< @brief Min RX buffer length */
#define	__SPI_MINTXBUFLEN		32		/*!< @brief Min TX buffer length */

/**
  * @}
  */

/** @defgroup SPI_PlatIoCtlCodesDefines Platform IOCTLs
  *
  * The following values will be called from the SPI driver in order
  * to give the platform control over the SPI device hardware.
  * Users that may implement the SPI platform driver must create
  * a plat_spi.h (and plat_spi.c) file to manage the following
  * IO control codes with a @ref __DEV_PLAT_IOCTL type function
  * (\c dv_plat_ioctl member of the @ref __DEVICE structure). See
  * the __spiPlatIoCtl() example function.
  *
  * The @ref __DEV_PLAT_IOCTL function should return an appropriate
  * error code like the following declared in @ref Device_ErrorCodes.
  *
  * @arg __DEV_OK 			Operation succeeded.
  * @arg __DEV_ERROR		Error detected.
  * @arg __DEV_WRITE_ERROR	Write error detected.
  * @arg __DEV_READ_ERROR 	Read error detected.
  * @arg __DEV_UNK_IOCTL 	The IO control code passed in the \c code argument
  * 						is unknown.
  * @arg __DEV_MEM_ERROR 	Memory error.
  * @arg __DEV_TIMEOUT 		Operation timeout.
  * @arg __DEV_BUSY			Device is busy.
  *
  * The return code for success is usually __DEV_OK unless another error code
  * is specified in the list below.
  * @{
  */

/*!
 * @brief Called from __spiOpen() to initialize the hardware.
 */
#define __SPI_PLAT_INIT_HW			1

/*!
 * @brief De-initialize the hardware. Called from __spiDeinit().
 */
#define __SPI_PLAT_DEINIT_HW		2

/*!
 * @brief Sets the device speed in Hz. The speed value will be passed
 * in the \c param parameter of the @ref __DEV_PLAT_IOCTL function.
 */
#define __SPI_PLAT_SET_SPEED		3

/*!
 * @brief Writes a character (1 byte) to the SPI device. The character will
 * be passed in the \c param parameter of the @ref __DEV_PLAT_IOCTL function.
 */
#define __SPI_PLAT_CHAR_OUTPUT		4

/*!
 * @brief Reads a character (1 byte) from the SPI device. The read character
 * has to be returned from of the @ref __DEV_PLAT_IOCTL function.
 */
#define __SPI_PLAT_CHAR_INPUT		5

/*!
 * @brief Start transmission.
 */
#define __SPI_PLAT_INIT_TX			6

/*!
 * @brief End transmission.
 */
#define __SPI_PLAT_END_TX			7

/*!
 * @brief Configure and/or set interrupts.
 */
#define __SPI_PLAT_SET_IRQ			8

/*!
 * @brief Reset interrupts.
 */
#define __SPI_PLAT_RESET_IRQ		9

/*!
 * @brief Set CS pin high or low. The pin status will be passed as
 * a boolean value in the \c param parameter of the @ref __DEV_PLAT_IOCTL function.
 */
#define __SPI_PLAT_SET_CS			10

/*!
 * @brief Called by __spiInit() when the driver requests the platform to initialize the
 * default values and devices.
 */
#define __SPI_PLAT_INIT_DEFAULTS	11

/**
  * @}
  */

/** @defgroup SPI_ModesDefines Modes
  * @{
  */

#define __SPIMODE_MASTER			0x01	/*!< @brief Master mode */
#define __SPIMODE_SLAVE				0x02	/*!< @brief Slave mode */

/**
  * @}
  */

/** @defgroup SPI_ErrorCodesDefines Error codes
  * @{
  */

#define	__SPIERR_OVERFLOW		0x01	/*!< @brief Receiver buffer overflow */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup SPI_TypeDefs Typedefs
  * @{
  */

typedef struct	__spipdbTag {

	u8				pd_flags;		/*!< @brief Buffer type (static or dynamic) */
	__VOLATILE u8	pd_rxerr;		/*!< @brief Receiver error */
	__VOLATILE u16	pd_rxtmo;		/*!< @brief Receiver timeout */
	__VOLATILE u16	pd_rcidx;		/*!< @brief Receiver buffer index */
	__VOLATILE u16	pd_rbidx;		/*!< @brief Read buffer index */
	__VOLATILE u16	pd_rxcnt;		/*!< @brief Received chars count */
	__VOLATILE u16	pd_rxrev;		/*!< @brief Received chars count reverse count */
	__VOLATILE u16	pd_rxlen;		/*!< @brief RX buffer length */
	pu8				pd_rxbuf;		/*!< @brief RX buffer */
	__VOLATILE u16	pd_txtmo;		/*!< @brief Transmit timeout */
	__VOLATILE u16	pd_tcidx;		/*!< @brief Transmit buffer index */
	__VOLATILE u16	pd_tbidx;		/*!< @brief Write buffer index */
	__VOLATILE u16	pd_txcnt;		/*!< @brief Transmit chars count */
	__VOLATILE u16	pd_txlen;		/*!< @brief TX buffer length */
	pu8				pd_txbuf;		/*!< @brief TX buffer */
	u8				pd_asscs;		/*!< @brief CS pin assert. If 1, CS pin will be managed. */
	u8				pd_mode;		/*!< @brief Master or slave. */

} __SPI_PDB, *__PSPI_PDB;

typedef struct __spiConfig
{
	u16		rxsize;
	u16		txsize;

} __SPI_CONFIG, *__PSPI_CONFIG;


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

i32 __spiInit(__PDEVICE dv, __PVOID params);
i32 __spiDeinit(__PDEVICE dv);
i32 __spiIOCtl(__PDEVICE dv, u32 cmd, u32 param, __PVOID data, u32 len);
i32 __spiOpen(__PDEVICE dv, u32 mode);
i32 __spiClose(__PDEVICE dv);
i32 __spiSize(__PDEVICE dv, u8 mode);
i32 __spiFlush(__PDEVICE dv);
i32 __spiRead(__PDEVICE dv, __PVOID buf, u16 qty);
i32 __spiWrite(__PDEVICE dv, __CONST __PVOID buf, u16 qty);

#endif // __SPI_H__

