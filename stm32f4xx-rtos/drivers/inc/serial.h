/***************************************************************************
 * serial.h
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

#ifndef	__SERIAL_H__
#define	__SERIAL_H__

#include <core/inc/system.h>
#include <core/inc/intrvect.h>
#include <core/inc/device.h>
#include <plat_uart.h>

/** @addtogroup Driver
  * @{
  */

/** @addtogroup Serial
  * @{
  */

/** @defgroup Serial_Constants Constants
  * @{
  */

/** @defgroup Serial_ConfigurationConstants Configuration
  * @{
  */

/** @defgroup Serial_BitsDefines Stop bits
  * @{
  */

#define	__SERIAL_STOPBITS_1			0x01
#define	__SERIAL_STOPBITS_0_5		0x02
#define	__SERIAL_STOPBITS_2			0x04
#define	__SERIAL_STOPBITS_1_5		0x08
#define	__SERIAL_STOPBITS_MASK		0x0F

/**
  * @}
  */

/** @defgroup Serial_ParityDefines Parity
  * @{
  */

#define	__SERIAL_PARITY_NO			0x10
#define	__SERIAL_PARITY_EVEN		0x20
#define	__SERIAL_PARITY_ODD			0x40
#define __SERIAL_PARITY_MASK		0x70

/**
  * @}
  */

/** @defgroup Serial_FlowControlDefines Flow control
  * @{
  */

#define	__SERIAL_FLOW_NONE			0x80
#define	__SERIAL_FLOW_RTS			0x100
#define	__SERIAL_FLOW_CTS			0x200
#define __SERIAL_FLOW_RTS_CTS		0x400
#define __SERIAL_FLOW_MASK			0x780
/**
  * @}
  */

/** @defgroup Serial_LengthDefines Length
  * @{
  */

#define	__SERIAL_LENGTH_8			0x800
#define	__SERIAL_LENGTH_9			0x1000
#define	__SERIAL_LENGTH_MASK		0x1800

/**
  * @}
  */

/** @defgroup Serial_Preconfigured Preconfigured
  * @{
  */

#define __SERIAL_8_N_1		(__SERIAL_LENGTH_8 | __SERIAL_FLOW_NONE | \
							__SERIAL_PARITY_NO | __SERIAL_STOPBITS_1)

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup Serial_MinimumBufferDefines Buffer
  * @{
  */

#define	__SERIAL_MINRXBUFLEN	__PLATUART_RXBUFF_LEN		/*!< @brief Min RX buffer length */
#define	__SERIAL_MINTXBUFLEN	__PLATUART_TXBUFF_LEN		/*!< @brief Min TX buffer length */

/**
  * @}
  */


/** @defgroup Serial_PlatIoCtlCodesDefines Platform IOCTLs
  *
  * The following values will be called from the Serial driver in order
  * to give the platform control over the Serial device hardware.
  * Users that may implement the Serial platform driver must create
  * a plat_uart.h (and plat_uart.c) file to manage the following
  * IO control codes with a @ref __DEV_PLAT_IOCTL type function
  * (\c dv_plat_ioctl member of the @ref __DEVICE structure). See
  * the __serialPlatIoCtl() example function.
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
 * @brief Called from __serialOpen() to initialize the hardware.
 */
#define __SERIAL_PLAT_INIT_HW			1

/*!
 * @brief De-initialize the hardware. Called from __serialDeinit().
 */
#define __SERIAL_PLAT_DEINIT_HW			2

/*!
 * @brief Set the device baud rate. The baud rate value will be passed
 * in the \c param parameter of the @ref __DEV_PLAT_IOCTL function.
 */
#define __SERIAL_PLAT_SET_BAUDRATE		3

/*!
 * @brief Writes a character (1 byte) to the Serial interface. The character will
 * be passed in the \c param parameter of the @ref __DEV_PLAT_IOCTL function.
 */
#define __SERIAL_PLAT_CHAR_OUTPUT		4

/*!
 * @brief Reads a character (1 byte) from the Serial interface. The read character
 * has to be returned from of the @ref __DEV_PLAT_IOCTL function.
 */
#define __SERIAL_PLAT_CHAR_INPUT		5

/*!
 * @brief Start transmission.
 */
#define __SERIAL_PLAT_INIT_TX			6

/*!
 * @brief Configure and/or set interrupts.
 */
#define __SERIAL_PLAT_SET_IRQ			7

/*!
 * @brief Reset interrupts.
 */
#define __SERIAL_PLAT_RESET_IRQ			8

/*!
 * @brief Set parity. The parity value can be any of the @ref Serial_ParityDefines
 * codes, passed in the \c param parameter of the @ref __DEV_PLAT_IOCTL function.
 */
#define __SERIAL_PLAT_SET_PARITY		9

/*!
 * @brief Set stop bit. The stop bit value can be any of the @ref Serial_BitsDefines
 * codes, passed in the \c param parameter of the @ref __DEV_PLAT_IOCTL function.
 */
#define __SERIAL_PLAT_SET_STOP_BITS		10

/*!
 * @brief Set HW flow control. The flow control value can be any of the
 * @ref Serial_FlowControlDefines codes, passed in the \c param parameter of the
 * @ref __DEV_PLAT_IOCTL function.
 */
#define __SERIAL_PLAT_SET_FLOW_CONTROL	11

/*!
 * @brief Set bit length. The length value can be any of the @ref Serial_LengthDefines
 * codes, passed in the \c param parameter of the @ref __DEV_PLAT_IOCTL function.
 */
#define __SERIAL_PLAT_SET_LENGTH		12

/**
  * @}
  */


/** @defgroup Serial_ErrorCodesDefines Error codes
  * @{
  */

#define	__SERIALERR_OVERFLOW		0x01	/*!< @brief Receiver buffer overflow */
#define	__SERIALERR_OVERRUN			0x02	/*!< @brief Overrun error */
#define	__SERIALERR_PARITY			0x02	/*!< @brief Parity error */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup Serial_TypeDefs Typedefs
  * @{
  */

typedef struct	__serialpdbTag {

	u8				pd_echo;		/*!< @brief Echo enabled-disabled */
	u32				pd_baud;		/*!< @brief Current baud rate */
	u32				pd_mode;		/*!< @brief Current mode */
	__VOLATILE u8	pd_rxerr;		/*!< @brief Receiver error */
	__VOLATILE u16	pd_rxtmo;		/*!< @brief Receiver timeout */
	__VOLATILE u16	pd_rcidx;		/*!< @brief Receiver buffer index */
	__VOLATILE u16	pd_rbidx;		/*!< @brief Read buffer index */
	__VOLATILE u16	pd_rxcnt;		/*!< @brief Received chars count */
	__VOLATILE u16	pd_rxrev;		/*!< @brief Received chars count reverse count */
	__VOLATILE u16	pd_rxlen;		/*!< @brief RX buffer length */
	pu8				pd_rxbuf;		/*!< @brief RX buffer */
	__VOLATILE u8	pd_txsnd;		/*!< @brief Transmission is started */
	__VOLATILE u16	pd_txtmo;		/*!< @brief Transmit timeout */
	__VOLATILE u16	pd_tcidx;		/*!< @brief Transmit buffer index */
	__VOLATILE u16	pd_tbidx;		/*!< @brief Write buffer index */
	__VOLATILE u16	pd_txcnt;		/*!< @brief Transmit chars count */
	__VOLATILE u16	pd_txlen;		/*!< @brief TX buffer length */
	pu8				pd_txbuf;		/*!< @brief TX buffer */

} __SERIAL_PDB, *__PSERIAL_PDB;

typedef struct __serialConfig
{
	u16		rxsize;
	u16		txsize;
} __SERIAL_CONFIG, *__PSERIAL_CONFIG;

/**
  * @}
  */


i32 __serialInit(__PDEVICE dv, __PVOID params);
i32 __serialDeinit(__PDEVICE dv);
i32 __serialIOCtl(__PDEVICE dv, u32 cmd, u32 param, __PVOID data, u32 len);
i32 __serialOpen(__PDEVICE dv, u32 mode);
i32 __serialClose(__PDEVICE dv);
i32 __serialSize(__PDEVICE dv, u8 mode);
i32 __serialFlush(__PDEVICE dv);
i32 __serialRead(__PDEVICE dv, __PVOID buf, u16 qty);
i32 __serialWrite(__PDEVICE dv, __CONST __PVOID buf, u16 qty);

/**
  * @}
  */

/**
  * @}
  */

#endif	// __SERIAL_H__
