/***************************************************************************
 * serial.c
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

#include "serial.h"
#include <core/inc/heap.h>
#include <common/inc/common.h>

#if __CONFIG_COMPILE_SERIAL


/** @addtogroup Driver
  * @{
  */

/** @defgroup Serial Serial
  * Serial device driver
  * @{
  */

/** @defgroup Serial_Functions Functions
  * @{
  */

/*!
 * @brief Device Input/Output control function.
 *
 * Called from __deviceIOCtl().
 *
 * @param	dv			Pointer to a device.
 * @param 	cmd			Command code to execute. One of the following values:
 *
 * @arg __IOCTL_SETSPEED
 * 		Sets the baudrate from the \c param parameter.
 *
 * @arg __IOCTL_GETSPEED
 * 		Writes the current baudrate in the \c data parameter.
 *
 * @arg __IOCTL_SETRXTIMEOUT
 * 		Sets the RX timeout from the \c param parameter.
 *
 * @arg __IOCTL_SETTXTIMEOUT
 * 		Sets the TX timeout from the \c param parameter.
 *
 * @arg __IOCTL_ECHO_ONOFF
 * 		Enables or disables the character echo on the serial port (\c param = non-zero
 * 		to enable echo, \c param = 0 to disable echo).
 *
 * @arg	__IOCTL_ECHO
 * 		Returns non-zero if the echo is enabled, otherwise 0.
 *
 * @arg __IOCTL_SETSTOPBITS
 * 		Set the stop bits. The value will be read from the \c param parameter and can be any
 * 		of the @ref Serial_BitsDefines values.
 *
 * @arg __IOCTL_SETPARITY
 * 		Set the parity. The value will be read from the \c param parameter and can be any
 * 		of the @ref Serial_ParityDefines values.
 *
 * @arg __IOCTL_SETBITLENGHT
 * 		Set the bit length. The value will be read from the \c param parameter and can be any
 * 		of the @ref Serial_LengthDefines values.
 *
 * @arg __IOCTL_SETFLOWCONTROL
 * 		Set the flow control. The value will be read from the \c param parameter and can be any
 * 		of the @ref Serial_FlowControlDefines values.
 *
 * @param	param		Input parameter.
 * @param	data		Optional data pointer.
 * @param 	len			Length of \c data.
 *
 * @return				__DEV_OK on success, __DEV_ERROR on error, __DEV_UNK_IOCTL if the
 * 						IO control code is not implemented.
 *
 */
i32	__serialIOCtl(__PDEVICE dv, u32 cmd, u32 param, __PVOID data, u32 len)
{
	pu32 lpar = data;
	__PSERIAL_PDB	pd = dv->dv_pdb;
	
	switch(cmd)
	{
		case __IOCTL_SETSPEED:
			(dv->dv_plat_ioctl)(dv,	__SERIAL_PLAT_SET_BAUDRATE, param,
								__NULL, 0, __NULL, 0);
			pd->pd_baud = param;
			return __DEV_OK;

		case __IOCTL_GETSPEED:
			*lpar = pd->pd_baud = param;
			return __DEV_OK;

		case __IOCTL_SETRXTIMEOUT:
			pd->pd_rxtmo = (u16) param;
			return __DEV_OK;

		case __IOCTL_SETTXTIMEOUT:
			pd->pd_txtmo = (u16) param;
			return __DEV_OK;

		case __IOCTL_ECHO_ONOFF:
			pd->pd_echo = (u8) param;
			return __DEV_OK;

		case __IOCTL_ECHO:
			return pd->pd_echo;

		case __IOCTL_SETSTOPBITS:
			pd->pd_mode &= ~__SERIAL_STOPBITS_MASK;
			pd->pd_mode |= param;

			return ((dv->dv_plat_ioctl)
					(dv,	__SERIAL_PLAT_SET_STOP_BITS, param, __NULL, 0, __NULL, 0));

		case __IOCTL_SETPARITY:
			pd->pd_mode &= ~__SERIAL_PARITY_MASK;
			pd->pd_mode |= param;

			return ((dv->dv_plat_ioctl)
					(dv, __SERIAL_PLAT_SET_PARITY, param, __NULL, 0, __NULL, 0));

		case __IOCTL_SETBITLENGHT:
			pd->pd_mode &= ~__SERIAL_LENGTH_MASK;
			pd->pd_mode |= param;

			return ((dv->dv_plat_ioctl)
					(dv, __SERIAL_PLAT_SET_LENGTH, param, __NULL, 0, __NULL, 0));

		case __IOCTL_SETFLOWCONTROL:
			pd->pd_mode &= ~__SERIAL_FLOW_MASK;
			pd->pd_mode |= param;

			return ((dv->dv_plat_ioctl)
					(dv, __SERIAL_PLAT_SET_FLOW_CONTROL, param, __NULL, 0, __NULL, 0));

	}

	return __DEV_UNK_IOCTL;
}

/*!
 * @brief Initialization.
 *
 * Called from __deviceInit() to initialize the serial device driver.
 *
 * @param	dv			Pointer to a device.
 * @param 	params		Pointer to __SERIAL_CONFIG structure.
 * @return				__DEV_OK on success, otherwise __DEV_ERROR.
 *
 */
i32 __serialInit(__PDEVICE dv,  __PVOID params)
{
	__PSERIAL_PDB pd = dv->dv_pdb;
	__PSERIAL_CONFIG config = (__PSERIAL_CONFIG) params;

	if (config == __NULL)
	{
		/* Use default values */
		pd->pd_rxlen = __SERIAL_MINRXBUFLEN;
		pd->pd_txlen = __SERIAL_MINTXBUFLEN;
	} else {
		pd->pd_rxlen = config->rxsize;
		pd->pd_txlen = config->txsize;
	}

	if (pd->pd_rxlen < __SERIAL_MINRXBUFLEN) pd->pd_rxlen = __SERIAL_MINRXBUFLEN;
	if (pd->pd_txlen < __SERIAL_MINTXBUFLEN) pd->pd_txlen = __SERIAL_MINTXBUFLEN;
	
	if (dv->dv_rxev) __memSet(dv->dv_rxev, 0, sizeof(__EVENT));
	if (dv->dv_txev) __memSet(dv->dv_txev, 0, sizeof(__EVENT));

	if ((pd->pd_rxbuf = __heapAlloc(pd->pd_rxlen)) == __NULL) return __DEV_ERROR;
	if ((pd->pd_txbuf = __heapAlloc(pd->pd_txlen)) == __NULL)
	{
		__heapFree(pd->pd_rxbuf);
		return __DEV_ERROR;
	}
	
	return __DEV_OK;
}

/*!
 * @brief Serial device driver de-initialization.
 *
 * Called from __deviceDeinit().
 *
 * @param	dv			Pointer to a device.
 * @return				__DEV_OK on success, otherwise __DEV_ERROR.
 *
 */
i32	__serialDeinit(__PDEVICE dv)
{
	__PSERIAL_PDB	pd = dv->dv_pdb;

	/* Check if still opened */
	if (dv->dv_opcnt != 0) return __DEV_ERROR;

	if (pd != __NULL) {
		if (pd->pd_rxbuf != __NULL) __heapFree(pd->pd_rxbuf);
		if (pd->pd_txbuf != __NULL) __heapFree(pd->pd_txbuf);
	}
	return __DEV_OK;
}

/*!
 * @brief Serial device driver open function.
 *
 * Called from __deviceOpen() to open the serial device.
 *
 * @param	dv			Pointer to a device.
 * @param 	baudrate	Desired baud rate.
 *
 * @return	__DEV_OK on success, otherwise __DEV_ERROR.
 *
 */
i32	__serialOpen(__PDEVICE dv, u32 baudrate)
{
	__PSERIAL_PDB pd = dv->dv_pdb;

	/* Platform should configure the UART with the pd->pd_mode value */
	pd->pd_mode = __SERIAL_8_N_1;
	pd->pd_baud = baudrate;

	/*	Init hardware */
	if (((dv->dv_plat_ioctl)(dv, __SERIAL_PLAT_INIT_HW,
		0, __NULL, 0, __NULL, 0)) != __DEV_OK) return __DEV_ERROR;

	/* Set TX/RX interrupt vector */
	if  (((dv->dv_plat_ioctl)(dv,	__SERIAL_PLAT_SET_IRQ,
		0, __NULL, 0, __NULL, 0)) != __DEV_OK) return __DEV_ERROR;

	return __DEV_OK;
}

/*!
 * @brief Serial device driver close function.
 *
 * Called from __deviceClose() to close the serial device.
 *
 * @param	dv			Pointer to a device.
 * @return	__DEV_OK on success, otherwise __DEV_ERROR.
 *
 */
i32	__serialClose(__PDEVICE dv)
{
	/*	De-init hardware */
	(dv->dv_plat_ioctl)(dv,	__SERIAL_PLAT_DEINIT_HW,
							0, __NULL, 0, __NULL, 0);

	/* Disable TX/RX interrupt vector */
	(dv->dv_plat_ioctl)(dv,	__SERIAL_PLAT_RESET_IRQ,
						0, __NULL, 0, __NULL, 0);

	return __DEV_OK;
}

/*!
 * @brief Returns the count of the serial unsent/unread bytes.
 *
 * Called from __deviceSize().
 *
 * @param	dv				Pointer to a device.
 * @param 	mode			Parameter defining on which buffer operate.
 * @arg		__DEV_RXSIZE	Get the RX buffer unread bytes.
 * @arg		__DEV_TXSIZE	Get the TX buffer unsent bytes.
 * @return					the serial unsent/unread bytes. -1 on error.
 *
 */
i32	__serialSize(__PDEVICE dv, u8 mode)
{
	__PSERIAL_PDB	pd = dv->dv_pdb;

	if (mode == __DEV_RXSIZE) return(pd->pd_rxcnt);
	if (mode == __DEV_TXSIZE) return(pd->pd_txcnt);
	return __DEV_ERROR;
}


/*!
 * @brief Serial device driver read function.
 *
 * Read from the RX buffer if there is data available, otherwise it will wait
 * on an event until receives data. Called from __deviceRead().
 *
 * @param	dv			Pointer to a device.
 * @param 	buf			Pointer to a buffer to receive the data.
 * @param	qty			Required quantity to read.
 * @return				Bytes read or -1 on error.
 *
 */
i32	__serialRead(__PDEVICE dv, __PVOID buf, u16 qty)
{
	u16	cnt = 0;
	__PSTRING	p = buf;
	__PSERIAL_PDB	pd = dv->dv_pdb;

	while (cnt < qty)
	{
		if (pd->pd_rxcnt > 0)
		{
			*p++ = *(pd->pd_rxbuf + pd->pd_rbidx);
			if (++pd->pd_rbidx >= pd->pd_rxlen) pd->pd_rbidx = 0;
			--pd->pd_rxcnt;
			++cnt;
		} else
		{
			if (!dv->dv_rxev) return cnt;
			__eventReset(dv->dv_rxev);
			if (__eventWait(dv->dv_rxev,(u32) pd->pd_rxtmo) == __EVRET_TIMEOUT)
			{
				return cnt;
			}
		}
	}
	return cnt;
}

/*!
 * @brief Serial device driver flush function.
 *
 * Flush remaining unsent bytes through the UART. Called from __deviceFlush().
 *
 * @param	dv			Pointer to a device.
 * @return				__DEV_OK on success, __DEV_TIMEOUT on timeout.
 *
 */
i32	__serialFlush(__PDEVICE dv)
{
	__PSERIAL_PDB pd = dv->dv_pdb;

	/* something to send? */
	if (!pd->pd_txcnt) return __DEV_OK;

	if (dv->dv_txev) __eventReset(dv->dv_txev);

	/*	Init transmission */
	(dv->dv_plat_ioctl)(dv,	__SERIAL_PLAT_INIT_TX,
						0, __NULL, 0, __NULL, 0);

	if (!dv->dv_txev) return 0;
	if (__eventWait(dv->dv_txev, pd->pd_txtmo) == __EVRET_SUCCESS) return __DEV_OK;

	return __DEV_TIMEOUT;
}

/*!
 * @brief Serial device driver write function.
 *
 * Writes to the TX buffer. Called from __deviceWrite().
 *
 * @param	dv			Pointer to a device.
 * @param	buf			Pointer to the buffer containing data.
 * @param	qty			Quantity of bytes to write.
 * @return				Bytes written or -1 on error.
 *
 */
i32	__serialWrite(__PDEVICE dv, __CONST __PVOID buf, u16 qty)
{
	u16 cnt = 0;
	u8* p = (u8*) buf;
	__PSERIAL_PDB pd = dv->dv_pdb;

	while (cnt < qty)
	{
		if (pd->pd_txcnt < pd->pd_txlen)
		{
			*(pd->pd_txbuf + pd->pd_tbidx) = *p++;
			if (++pd->pd_tbidx >= pd->pd_txlen) pd->pd_tbidx = 0;
			++pd->pd_txcnt;
			++cnt;
		} else
		{
			if (__serialFlush(dv) != __DEV_OK)
			{
				return __DEV_ERROR;
			}
		}
	}
	return cnt;
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

#endif // __CONFIG_COMPILE_SERIAL

