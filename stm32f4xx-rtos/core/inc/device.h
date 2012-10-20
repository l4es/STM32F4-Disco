/***************************************************************************
 * device.h
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

#ifndef	__DEVICE_H__
#define	__DEVICE_H__

#include <plat_ostypes.h>
#include "ioctl.h"
#include "event.h"
#include "thread.h"

/** @addtogroup Core
  * @{
  */

/** @addtogroup Device
  * @{
  */

/** @defgroup Device_Constants Constants
  * @{
  */

/** @addtogroup Device_PrivateDefines
  * Internal defines.
  * @{
  */

#define	__DEV_NAMELEN		8		/*!< @brief Device name maximum length. */

/**
  * @}
  */

/** @defgroup Device_ModesDefines Modes
  * @brief Values used by __deviceInit() function.
  *
  * Describes the device modes, used in __deviceInit() function, parameter \c mode.
  * @{
  */

#define	__DEV_RD_CR			0x01	/*!< @brief Read CR flag */
#define	__DEV_RD_LF			0x02	/*!< @brief Read LF flag */
#define	__DEV_RD_CRLF		0x03	/*!< @brief Read CR/LF flag */
#define	__DEV_WR_CR			0x04	/*!< @brief Write CR flag */
#define	__DEV_WR_LF			0x08	/*!< @brief Write LF flag */
#define	__DEV_AUTOFLUSH		0x10	/*!< @brief Flush after write. */
#define	__DEV_WR_CRLF		0x0C	/*!< @brief Write CR/LF flag */

/**
  * @}
  */

/** @defgroup Device_ErrorCodes Error codes
  * @brief Values used by the @ref Device module @ref Device_Functions.
  *
  * Callback functions may return any of the following codes.
  * This is an unsigned 32-bits value.
  * The implemented devices may add/return their own error codes.
  * @{
  */

#define __DEV_OK			0
#define __DEV_ERROR			-1
#define __DEV_WRITE_ERROR	-2
#define __DEV_READ_ERROR	-3
#define __DEV_UNK_IOCTL		-4
#define __DEV_MEM_ERROR		-5
#define __DEV_TIMEOUT		-6
#define __DEV_BUSY			-7
#define __DEV_INVALID		-8
#define __DEV_NOT_READY		-9

/**
  * @}
  */

/** @defgroup Device_SizeParamDefines Size
  * @brief Values used by __deviceSize() function.
  *
  * Used in __deviceSize() function to retrieve the size of the read or write buffer.
  *
  * @{
  */
#define	__DEV_RXSIZE		0		/*!< @brief Retrieves the size of the RX buffer */
#define	__DEV_TXSIZE		1		/*!< @brief Retrieves the size of the TX buffer */

/**
  * @}
  */

/** @defgroup Device_TypesDefines Device types
  * @brief Values used when defining a __DEVICE structure.
  * @{
  */
#define	__DEV_STREAM		1		/*!< @brief Device type STREAM */
#define	__DEV_BLOCK			2		/*!< @brief Device type BLOCK */
#define	__DEV_ETHER			3		/*!< @brief Device type NETWORK */
#define	__DEV_PPP			4		/*!< @brief Device type PPP */
#define	__DEV_DISK			5		/*!< @brief Device type DISK */
#define	__DEV_IRDA			6		/*!< @brief Device type IRDA */
#define	__DEV_BTH			7		/*!< @brief Device type BLUETOOTH */
#define	__DEV_ADC			8		/*!< @brief Device type A/D converter */
#define	__DEV_USART			9		/*!< @brief Device type USART */
#define	__DEV_SPI			10		/*!< @brief Device type SPI */
#define	__DEV_DISPLAY		11		/*!< @brief Device type display */
#define	__DEV_I2C			12		/*!< @brief Device type I2C */
#define	__DEV_SDIO			13		/*!< @brief Device type SDIO */
#define	__DEV_USBH			14		/*!< @brief Device type USB host */
#define	__DEV_USBD			15		/*!< @brief Device type USB device */
#define	__DEV_USB_OTG		16		/*!< @brief Device type USB on-the-go */
#define __DEV_CFSD			17		/*!< @brief Device type Compact flash/SD */
#define __DEV_EEPROM		18		/*!< @brief Device type EEPROM memory */
#define __DEV_SOCKET		19		/*!< @brief Device type network socket */

/**
  * @}
  */

/**
  * @}
  */

struct __deviceTag;

/** @defgroup Device_FunctionPrototypes Function prototypes
  * @{
  */

/*!
 * @brief Initializes the device
 */
typedef	i32	(__DEV_INIT)(struct __deviceTag*, __PVOID);

/*!
 * @brief De-initializes the device
 */
typedef	i32	(__DEV_DEINIT)(struct __deviceTag*);

/*!
 * @brief IOCTL function
 */
typedef	i32	(__DEV_IOCTL)(struct __deviceTag*, u32, u32, __PVOID, u32);

/*!
 * @brief Opens the device
 */
typedef	i32 (__DEV_OPEN)(struct __deviceTag*, u32);

/*!
 * @brief Closes the device
 */
typedef	i32	(__DEV_CLOSE)(struct __deviceTag*);

/*!
 * @brief Reads from RX buffer
 */
typedef	i32	(__DEV_READ)(struct __deviceTag*, __PVOID, u16);

/*!
 * @brief Writes to TX buffer
 */
typedef	i32	(__DEV_WRITE)(struct __deviceTag*, __CONST __PVOID, u16);

/*!
 * @brief Flushes TX buffer
 */
typedef	i32	(__DEV_FLUSH)(struct __deviceTag*);

/*!
 * @brief Retrieves TX/RX buffer size
 */
typedef	i32	(__DEV_SIZE)(struct __deviceTag*, u8);

/*!
 * @brief Platform-related IO control
 */
typedef i32 (__DEV_PLAT_IOCTL)(struct __deviceTag*, u32, u32, __PVOID, u32, __PVOID, u32);

/**
  * @}
  */

/** @defgroup Device_TypeDefs Typedefs
  * @{
  */

/**
 * @brief Device driver control block.
 *
 * To use the Milos device driver schema, create a __DEVICE structure and fill
 * the required fields, providing the common interface functions. Most hardware drivers will
 * depend on the running platform, so it is a good practice to define them in the @ref Platform module.
 * See plat_uart.c, and serial.c for a device driver implementation example.
 *
 */
typedef struct __deviceTag {

	__STRING			dv_name[__DEV_NAMELEN + 1];	/*!< @brief Device name */
	u8					dv_type;					/*!< @brief Device type */
	u32					dv_flags;					/*!< @brief Device flags */
	u8					dv_rxint;					/*!< @brief RX interrupt */
	u8					dv_txint;					/*!< @brief TX interrupt */
	u8					dv_initd;					/*!< @brief Initialized flag */
	u8					dv_opcnt;					/*!< @brief Open count */
	__PEVENT			dv_rxev;					/*!< @brief RX event */
	__PEVENT			dv_txev;					/*!< @brief TX event */
	__PTHREAD			dv_owner;					/*!< @brief Thread owner */
	__PVOID				dv_pdb;						/*!< @brief Private Data Block */
	__PVOID				dv_params;					/*!< @brief Customizable parameters */
	struct __deviceTag*	dv_next;					/*!< @brief Next device in linked list */

	/* Device interface functions */
	__DEV_INIT			*dv_init;					/*!< @brief Initialize device function */
	__DEV_DEINIT		*dv_deinit;					/*!< @brief De-initialize device function */
	__DEV_IOCTL			*dv_ioctl;					/*!< @brief Device IO control function */
	__DEV_OPEN			*dv_open;					/*!< @brief Open device function */
	__DEV_CLOSE			*dv_close;					/*!< @brief Close device function */
	__DEV_READ			*dv_read;					/*!< @brief Read from device function */
	__DEV_WRITE			*dv_write;					/*!< @brief Write to device function */
	__DEV_FLUSH			*dv_flush;					/*!< @brief Flush device function */
	__DEV_SIZE			*dv_size;					/*!< @brief Get size function */
	__DEV_PLAT_IOCTL	*dv_plat_ioctl;				/*!< @brief Platform-related IO control */
} __DEVICE, *__PDEVICE;

/**
  * @}
  */


__PDEVICE __deviceGetList(__VOID);
__PDEVICE __deviceFind(__CONST __PSTRING name);
i32 __deviceInit(__PDEVICE dv, __PVOID params, u32 mode);
i32 __deviceDeinit(__PDEVICE dv);
i32 __deviceWriteLine(__PDEVICE dv, __CONST __PSTRING buf);
i32 __deviceReadLine(__PDEVICE dv, __PSTRING buf, u16 qty);
i32 __deviceIOCtl(__PDEVICE dv, u32 cmd, u32 param, __PVOID data, u32 len);
i32 __deviceOpen(__PDEVICE dv, u32 param);
i32 __deviceClose(__PDEVICE dv);
i32 __deviceRead(__PDEVICE dv, __PVOID buf, u16 qty);
i32 __deviceWrite(__PDEVICE dv, __CONST __PVOID buf, u16 qty);
i32 __deviceFlush(__PDEVICE dv);
i32 __deviceSize(__PDEVICE dv, u8 mode);

__VOID __deviceAdd(__PDEVICE dv, u8 count);
__VOID __deviceRemove(__PDEVICE dv);
__VOID __deviceDbgTermOutput(__VOID);

#define __deviceInitialized(dv)	(dv->dv_initd)
#define __deviceOpened(dv)		(dv->dv_opcnt)

/**
  * @}
  */

/**
  * @}
  */

#endif	/*__DEVICE_H__ */
