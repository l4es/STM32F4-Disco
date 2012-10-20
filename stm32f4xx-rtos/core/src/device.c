/***************************************************************************
 * device.c
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

#include "device.h"
#include <core/inc/system.h>
#include <core/inc/dbgterm.h>
#include <common/inc/string.h>

/** @addtogroup Core
  * @{
  */

/** @defgroup Device Devices
  * @brief Device drivers functions.
  *
  * This module defines the main functions for device drivers.
  * To implement a device driver fill a __DEVICE structure with the required
  * parameters, to be used with the functions described below.
  * Users may find appropriate to implement the device driver in two or more steps:
  * One step is to implement the common code that can be used in different platforms right in the
  * \\drives directory. Other steps may include the division of the common driver code from the
  * platform-specific code, and to declare the __DEVICE structure in the \\board directory inside the platform.
  * This way, for example, an application developer should find himself using "serial1" no matter the platform
  * or board he is using; because the serial code for that platform is surrounded with __deviceXXXXXX functions
  * that hides driver implementation, and by defining "serial1" on the \\board directory should guarantee that
  * switching boards for a specific development may still find the "serial1" device wherever is located on the
  * board.
  *
  * @{
  */

/** @defgroup Device_Constants Constants
  * @{
  */

/** @defgroup Device_PrivateDefines Private constants
  * Internal defines.
  * @{
  */

#define	__DEV_INITIALIZED	0x80	/*!< @brief Device init flag */
#define	__DEV_OPENED		0x40	/*!< @brief Device open flag */
#define	__DEV_EOL			0x00	/*!< @brief Line null terminator char */
#define	__DEV_CR			0x0D	/*!< @brief CR char */
#define	__DEV_LF			0x0A	/*!< @brief LF char */
#define	__DEV_RD_CRLFMASK	0x03	/*!< @brief Mask to obtain CR/LF read bits */
#define	__DEV_WR_CRLFMASK	0x0C	/*!< @brief Mask to obtain CR/LF read bits */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup Device_PrivateVariables Private variables
  * @{
  */

__STATIC __PDEVICE __deviceChain = __NULL;	/*!< @brief Device list chain */

/**
  * @}
  */


/** @defgroup Device_Functions Functions
  * @{
  */

/*!
 * @brief Find device by name.
 *
 * This function returns the address of the device specified in the \c name parameter.
 *
 * @param	name		Pointer to device name string.
 * @return				Pointer to a __DEVICE structure representing the device. __NULL if device
 * 						is not initialized.
 *
 */
__PDEVICE __deviceFind(__CONST __PSTRING name)
{
	__PDEVICE	dv = __deviceChain;

	while(dv)
	{
		if (__strCmp(dv->dv_name,name) == 0) return(dv);
		dv = dv->dv_next;
	}
	return(__NULL);
}


/*!
 * @brief Initializes a device.
 *
 * Call this function to initialize a device. It will generate a call to the
 * __DEV_INIT() function provided in the __DEVICE structure pointer.
 * The purpose of the __DEV_INIT() function will be to prepare the device to
 * be used with the rest of the @ref Device module functions.
 *
 * @param	dv			Pointer to valid __DEVICE device to initialize.
 *
 * @param	params		Void pointer to a configuration structure o device (depends on
 * 						the driver implementation) to be passed to the @ref
 * 						__DEV_INIT function.
 *
 * @param	flags		Optional flags (used by device). Can be a combination of
 * 						the following values:
 *
 * @arg __DEV_RD_CR 	When a CR is received, the __deviceReadLine() function
 * 						will add a NULL character and return the read line.
 *
 * @arg __DEV_RD_LF 	When a LF is received, the __deviceReadLine() function
 * 						will add a NULL character and return the read line.
 *
 * @arg __DEV_RD_CRLF 	The combination of __DEV_RD_CR and __DEV_RD_LF flags.
 *
 * @arg __DEV_WR_CR 	The __deviceWriteLine() function will transmit a CR
 * 						character after a successfully call to the __DEV_WRITE()
 * 						 function.
 *
 * @arg __DEV_WR_LF 	The __deviceWriteLine() function will transmit a LF
 * 						character after a successfully call to the __DEV_WRITE()
 * 						function.
 *
 * @arg __DEV_WR_CRLF 	The combination of __DEV_WR_CR and __DEV_WR_LF flags.
 *
 * @arg __DEV_AUTOFLUSH	The __deviceWrite() function will flush the TX
 * 						buffer immediately after a successfully call to the
 * 						__DEV_WRITE() function.
 *
 * @return	__DEV_OK on success, non-zero on failure. Error code may depend on
 * 			underlying device implementation.
 *
 */
i32	__deviceInit(__PDEVICE dv, __PVOID params, u32 flags)
{

	if (!dv) return __DEV_ERROR;

	/* Check if already initialized */
	if (!dv->dv_initd)
	{
		/* Check if device has an init function */
		if (dv->dv_init != __NULL)
		{
			if ((*dv->dv_init)(dv, params) != __DEV_OK) return __DEV_ERROR;
			dv->dv_initd = 1;
		}
	}

	dv->dv_flags = flags;
	return __DEV_OK;
}

/*!
 * @brief De-initializes a device.
 *
 * Call this function to de-initializes a device. It will generate a call to the
 * __DEV_DEINIT() function provided in the __DEVICE structure pointer.
 *
 * @param dv	Pointer to a valid device to de-initialize.
 *
 * @return		__DEV_OK on success, non-zero on failure. Error code may depend on underlying
 * 				device implementation.
 */
i32	__deviceDeinit(__PDEVICE dv)
{
	i32 ret;

	/* Check initialized */
	if (dv->dv_initd == 0) return __DEV_ERROR;

	/* Check if device has a destroy function */
	if (dv->dv_deinit == __NULL) {
		dv->dv_initd = 0;
		return __DEV_OK;
	}

	ret = (*dv->dv_deinit)(dv);

	if (ret == __DEV_OK) dv->dv_initd = 0;

	return ret;
}

/*!
 * @brief Opens a device.
 *
 * Opens a device. Must be called from a thread. It will generate a call to the
 * __DEV_OPEN() function provided in the __DEVICE structure pointer.
 *
 * @param dv	Pointer to valid device to open.
 * @param param	Optional value. It will be passed to the @ref __DEV_OPEN function.
 *
 * @return		__DEV_OK on success, non-zero on failure. Error code may depend on underlying
 * 				device implementation.
 */
i32	__deviceOpen(__PDEVICE dv, u32 param)
{
	i32 ret;

	/* Must be called from inside a thread */
	if (__threadGetCurrent()) {
		/* Check initialized */
		if (dv->dv_initd == 0) return __DEV_ERROR;

		/* Check if already opened */
		if (dv->dv_opcnt == 0)
		{
			/* Check for an open function */
			if (dv->dv_open != __NULL)
			{
				ret = (*dv->dv_open)(dv, param);

				if (ret == __DEV_OK)
				{
					dv->dv_owner = __threadGetCurrent();
					++dv->dv_opcnt;
				}

				return ret;
			}
		} else {
			++dv->dv_opcnt;
			return __DEV_OK;
		}
	}

	return __DEV_ERROR;
}

/*!
 * @brief Closes a device.
 *
 * Closes an opened device. It will generate a call to the __DEV_CLOSE()
 * function provided in the __DEVICE structure pointer.
 *
 * @param	dv		Pointer to valid device to close.
 *
 * @return			__DEV_OK on success, non-zero on failure. Error code may depend on underlying
 * 					device implementation.
 */
i32	__deviceClose(__PDEVICE dv)
{

	i32 ret;

	/* Must be called from within a thread */
	if (__threadGetCurrent())
	{
		if (dv->dv_opcnt == 1)
		{
			/* Check for a close function */
			if (dv->dv_close != __NULL)
			{
				ret = (*dv->dv_close)(dv);

				if (ret == __DEV_OK)
				{
					dv->dv_owner = __NULL;
					dv->dv_opcnt = 0;
				}

				return ret;
			}
		} else
		{
			--dv->dv_opcnt;
			return __DEV_OK;
		}
	}

	return __DEV_ERROR;
}

/*!
 * @brief Device IO Control function
 *
 * Generates a call to the __DEV_IOCTL() function provided in the __DEVICE
 * structure pointer.
 *
 * @param	dv		Pointer to valid device.
 * @param	cmd		Command code.
 * @param	param	Optional parameter value.
 * @param	data	Optional parameter pointer.
 * @param	len		Length of \c data.
 *
 * @return			__DEV_OK on success, non-zero on failure. Error code may depend on underlying
 * 					device implementation. __deviceIOCtl() may return custom values instead of error
 * 					codes. See the specific driver documentation.
 */
i32 __deviceIOCtl(__PDEVICE dv, u32 cmd, u32 param, __PVOID data, u32 len)
{
	/* Check for initialized */
	if (!dv->dv_initd) return __DEV_ERROR;

	if (dv->dv_ioctl) return((*dv->dv_ioctl)(dv,cmd,param,data,len));

	return __DEV_ERROR;
}

/*!
 * @brief Reads from device.
 *
 * Generates a call to the __DEV_READ() function provided in the __DEVICE
 * structure pointer.
 *
 * @param 	dv 		Pointer to valid device to read from.
 * @param 	buf		Buffer to receive device data.
 * @param 	qty		Amount of bytes to read.
 *
 * @return			Quantity of bytes read. A negative value on error. The
 * 					error code depends on the underlying driver implementation.
 * 					See the driver documentation.
 */
i32	__deviceRead(__PDEVICE dv, __PVOID buf, u16 qty)
{
	/* Check for initialized or opened */
	if (!dv->dv_initd || !dv->dv_opcnt) return __DEV_ERROR;

	if (dv->dv_owner != __NULL)
	{
		/* Check for a read function */
		if (dv->dv_read != __NULL)
		{
			return((*dv->dv_read)(dv,buf,qty));
		}
	}

	return __DEV_ERROR;
}

/*!
 * @brief Writes to a device.
 *
 * Generates a call to the __DEV_WRITE() function provided in the __DEVICE
 * structure pointer.
 *
 * @param 	dv		Pointer to a valid device to write to.
 * @param 	buf		Pointer to data to write.
 * @param 	qty		Amount of bytes to write.
 *
 * @return			Quantity of bytes written, otherwise an error code. The
 * 					error code depends on the underlying driver implementation.
 * 					See the driver documentation.
 */
i32	__deviceWrite(__PDEVICE dv, __CONST __PVOID buf, u16 qty)
{
	i32	res;

	/* Check for initialized or opened */
	if (!dv->dv_initd || !dv->dv_opcnt) return __DEV_ERROR;

	if (dv->dv_owner != __NULL)
	{
		if (dv->dv_write != __NULL)
		{
			res = ((*dv->dv_write)(dv,buf,qty));

			if (res > __DEV_OK) {
				if (dv->dv_flags & __DEV_AUTOFLUSH)
				{
					if (__deviceFlush(dv) == __DEV_OK)
						return res;
				}
			}

			return res;
		}
	}
	return __DEV_ERROR;
}

/*!
 * @brief Flushes device's TX buffer.
 *
 * Generates a call to the __DEV_FLUSH() function provided in the __DEVICE
 * structure pointer.
 * @param 	dv		Pointer to a valid device.
 * @return			__DEV_OK on success, otherwise an error code.
 */
i32	__deviceFlush(__PDEVICE dv)
{

	/* Check for initialized or opened */
	if (!dv->dv_initd || !dv->dv_opcnt) return __DEV_ERROR;

	if (dv->dv_owner != __NULL)
	{
		if (dv->dv_flush != __NULL)
		{
			return((*dv->dv_flush)(dv));
		}
	}

	return __DEV_ERROR;
}

/*!
 * @brief Get the remaining TX or RX bytes in the device's buffer.
 *
 * Generates a call to the __DEV_SIZE() function provided in the __DEVICE
 * structure pointer.
 *
 * @param	dv		Pointer to a valid device.
 * @param	mode	TX or RX buffer flag. One of the following values:
 * @arg __DEV_RXSIZE Retrieves the remaining unread bytes from the device's RX buffer.
 * @arg __DEV_TXSIZE Retrieves the remaining unsent bytes from the device's TX buffer.
 * @return			Bytes remaining or an error code.
 */
i32	__deviceSize(__PDEVICE dv, u8 mode)
{
	/* Check for initialized or opened */
	if (!dv->dv_initd || !dv->dv_opcnt) return __DEV_ERROR;

	if (dv->dv_owner != __NULL) {
		if (dv->dv_size != __NULL) {
			return((*dv->dv_size)(dv,mode));
		}
	}

	return __DEV_ERROR;
}

/*!
 * @brief Reads a line of text from the device.
 *
 * Calling the device's __DEV_READ() function it will scan for CR and/or LF characters
 * (depending on how the device was initialized (see the __deviceInit() function), and
 * will fill the provided buffer with a line of text.
 *
 * @param 	dv		Pointer to a valid device.
 * @param	buf		Buffer to receive the ASCII line.
 * @param	qty		Bytes to read.
 * @return 			Bytes read or an error code.
 */
i32 __deviceReadLine(__PDEVICE dv, __PSTRING buf, u16 qty)
{
	u16		i;
	u8		c, cr = 0;

	/* Check for initialized or opened */
	if (!dv->dv_initd || !dv->dv_opcnt) return __DEV_ERROR;

	if (dv == __NULL || dv->dv_read == __NULL || dv->dv_owner == __NULL) return __DEV_ERROR;

	for (i = 0; i < (qty - 1); i++)
	{
		if ((*dv->dv_read)(dv,&c,1) != 1)
		{
			*buf = __DEV_EOL;
			return __DEV_ERROR;
		}

		if (c == __DEV_CR || c == __DEV_LF)
		{
			--i;
			if (dv->dv_flags & __DEV_RD_CR)
			{
				if (!(dv->dv_flags &	__DEV_RD_LF))
				{
					*buf = __DEV_EOL;
					return i;
				}
				cr = c;
			}

			if (dv->dv_flags & __DEV_RD_LF)
			{
				if (!(dv->dv_flags &	__DEV_RD_CR))
				{
					*buf = __DEV_EOL;
					return i;
				}

				if (cr == __DEV_CR)
				{
					*buf = __DEV_EOL;
					return i;
				}
				cr = 0;
			}
		} else
		{
			*buf++ = c;
			cr = 0;
		}
	}

	return i;
}

/*!
 * @brief Writes a line of text to the device.
 *
 * After calling the device's __DEV_WRITE() function, it will send a CR and/or LF
 * characters (depending on how the device was initialized (see the __deviceInit()
 * function).
 *
 * @param 	dv		Pointer to a valid device.
 * @param	buf		The ASCII line of text to send.
 * @return 			Bytes sent or an error code.
 */
i32 __deviceWriteLine(__PDEVICE dv, __CONST __PSTRING buf)
{
	i16			qty = 0;
	u8			c;

	/* Check for initialized or opened */
	if (!dv->dv_initd || !dv->dv_opcnt) return __DEV_ERROR;

	if (dv == __NULL || dv->dv_write == __NULL || dv->dv_owner == __NULL) return __DEV_ERROR;

	if (buf == __NULL) return __DEV_ERROR;

	if ((*dv->dv_write)(dv, buf, __strLen(buf)) != __strLen(buf)) {
		return __DEV_ERROR;
	}
	
	qty = __strLen(buf);
	
	if (dv->dv_flags & __DEV_WR_CR) {
		c = __DEV_CR;
		if ((*dv->dv_write)(dv,&c,1) != 1)
		{
			return __DEV_ERROR;
		}
		++qty;
	}

	if (dv->dv_flags & __DEV_WR_LF)
	{
		c = __DEV_LF;
		if ((*dv->dv_write)(dv,&c,1) != 1)
		{
			return __DEV_ERROR;
		}
		++qty;
	}

	if (dv->dv_flags & __DEV_AUTOFLUSH)
	{
		if (dv->dv_flush) (*dv->dv_flush)(dv);
	}

	return qty;
}

/*!
 * @brief Add a device to the devices list.
 *
 * Usually called from @ref Platform module to preconfigure
 * available devices on a platform. Users may call __deviceFind() to
 * retrieve a handle to the device.
 *
 * @param 	dv		Pointer to a list of valid devices.
 * @param	count	Quantity of devices to add.
 * @return 			Nothing
 */
__VOID __deviceAdd(__PDEVICE dv, u8 count)
{
	u8 i;

	for (i = 0; i < count; i++)
	{
		dv->dv_next = __deviceChain;
		__deviceChain = dv;
		dv++;
	}
}

/*!
 * @brief Removes a device form the devices list.
 *
 * Removes a device added with the __deviceAdd() function.
 *
 * @param 	dv		Pointer to a device.
 *
 * @return 			Nothing
 */
__VOID __deviceRemove(__PDEVICE dv)
{
	__PDEVICE list;

	__systemDisableScheduler();

	list = __deviceChain;

	if (list == dv)
	{
		__deviceChain = dv->dv_next;
	} else {
		while (list)
		{

			if (list->dv_next == dv)
			{
				list->dv_next = dv->dv_next;
				break;
			}

			list = list->dv_next;
		}
	}

	__systemEnableScheduler();
}

/*!
 * @brief Returns the active devices list.
 */
__PDEVICE __deviceGetList(__VOID)
{
	return __deviceChain;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

