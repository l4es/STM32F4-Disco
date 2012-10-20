/***************************************************************************
 * i2c.c
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
#include "i2c.h"
#include <common/inc/mem.h>
#include <core/inc/heap.h>

#if __CONFIG_COMPILE_I2C

/** @addtogroup Driver
  * @{
  */

/** @defgroup I2C I2C
  * I2C device driver
  * @{
  */

/** @defgroup I2C_PrivateMacros Private macros
  * @{
  */

#define __i2cEnable()			((dv->dv_plat_ioctl) (dv, __I2C_PLAT_ENABLE_DEVICE, 0, __NULL, 0, __NULL, 0))
#define __i2cDisable()			((dv->dv_plat_ioctl) (dv, __I2C_PLAT_DISABLE_DEVICE, 0, __NULL, 0, __NULL, 0))
#define __i2cGenerateStart()	((dv->dv_plat_ioctl) (dv, __I2C_PLAT_GENERATE_START, 0, __NULL, 0, __NULL, 0))
#define __i2cGenerateStop()		((dv->dv_plat_ioctl) (dv, __I2C_PLAT_GENERATE_STOP, 0, __NULL, 0, __NULL, 0))
#define __i2cWaitForEvent(x)	((dv->dv_plat_ioctl) (dv, __I2C_PLAT_CHECK_EVENT, x, __NULL, 0, __NULL, 0))
#define __i2cAbort()			((dv->dv_plat_ioctl) (dv, __I2C_PLAT_ABORT, 0, __NULL, 0, __NULL, 0))
#define __i2cSendAddress(x)		((dv->dv_plat_ioctl) (dv, __I2C_PLAT_SET_ADDRESS, x, __NULL, 0, __NULL, 0))
#define __i2cSetOwnAddress(x)	((dv->dv_plat_ioctl) (dv, __I2C_PLAT_SET_OWN_ADDRESS, x, __NULL, 0, __NULL, 0))
#define __i2cSetAck(x)			((dv->dv_plat_ioctl) (dv, __I2C_PLAT_SET_ACK, x, __NULL, 0, __NULL, 0))
#define __i2cInitTx()			((dv->dv_plat_ioctl) (dv, __I2C_PLAT_INIT_TX, 0, __NULL, 0, __NULL, 0))
#define __i2cEndTx()			((dv->dv_plat_ioctl) (dv, __I2C_PLAT_END_TX, 0, __NULL, 0, __NULL, 0))
#define __i2cInitRx()			((dv->dv_plat_ioctl) (dv, __I2C_PLAT_INIT_RX, 0, __NULL, 0, __NULL, 0))
#define __i2cEndRx()			((dv->dv_plat_ioctl) (dv, __I2C_PLAT_END_RX, 0, __NULL, 0, __NULL, 0))
#define __i2cUsingIrq()			((dv->dv_plat_ioctl) (dv, __I2C_PLAT_USE_IRQ, 0, __NULL, 0, __NULL, 0))
#define __i2cSetAck(x)			((dv->dv_plat_ioctl) (dv, __I2C_PLAT_SET_ACK, x, __NULL, 0, __NULL, 0))
#define __i2cSendByte(x)		((dv->dv_plat_ioctl) (dv, __I2C_PLAT_WRITE_BYTE, x, __NULL, 0, __NULL, 0))
#define __i2cReadByte()			((dv->dv_plat_ioctl) (dv, __I2C_PLAT_READ_BYTE, 0, __NULL, 0, __NULL, 0))
#define __i2cCheckBusy()		if (pd->pd_busy) return __DEV_BUSY;
/**
  * @}
  */

/** @defgroup I2C_PrivateFunctions Private functions
  * @{
  */

/*!
 * @brief Toggles __TRUE/__FALSE the I2C internal busy flag.
 *
 * @param	dv		Pointer to the I2C device structure.
 * @param 	val		Boolean value to assign to the busy flag.
 *
 * @return Nothing.
 */
__STATIC __VOID __i2cSetBusy(__PDEVICE dv, __BOOL val)
{
	__PI2C_PDB pd = dv->dv_pdb;

	__systemStop();
	pd->pd_busy = val;
	__systemStart();
}


/**
  * @}
  */



/** @defgroup I2C_Functions Functions
  * @{
  */

/*!
 * @brief Initialization.
 *
 * Called from __deviceInit() to initialize the I2C device driver.
 *
 * @param	dv			Pointer to a device.
 * @param 	params		Pointer to a __I2C_CONFIG structure to configure the RX and TX
 * 						buffers. If left to __NULL the driver will take the default
 * 						values defined by the platform.
 *
 * @return				__DEV_OK on success, otherwise __DEV_ERROR.
 *
 */
i32 __i2cInit(__PDEVICE dv, __PVOID params)
{
	__PI2C_PDB pd = dv->dv_pdb;
	__PI2C_CONFIG config = (__PI2C_CONFIG) params;

	if (config == __NULL)
	{
		/* Load default values */
		pd->pd_rxlen = __I2C_MINRXBUFLEN;
		pd->pd_txlen = __I2C_MINTXBUFLEN;
	} else {
		pd->pd_rxlen = config->rxsize;
		pd->pd_txlen = config->txsize;
	}

	if (pd->pd_rxlen < __I2C_MINRXBUFLEN) pd->pd_rxlen = __I2C_MINRXBUFLEN;
	if (pd->pd_txlen < __I2C_MINTXBUFLEN) pd->pd_txlen = __I2C_MINTXBUFLEN;

	/* Clean the rx and tx events */
	if (dv->dv_rxev) __memSet(dv->dv_rxev, 0, sizeof(__EVENT));
	if (dv->dv_txev) __memSet(dv->dv_txev, 0, sizeof(__EVENT));

	/* Assign buffer size and alloc */
	if ((pd->pd_rxbuf = __heapAlloc(pd->pd_rxlen)) == __NULL) return __DEV_ERROR;
	if ((pd->pd_txbuf = __heapAlloc(pd->pd_txlen)) == __NULL)
	{
		__heapFree(pd->pd_rxbuf);
		return __DEV_ERROR;
	}

	return __DEV_OK;
}

/*!
 * @brief I2C device driver de-initialization.
 *
 * Called from __deviceDeinit() to de-initialize the I2C driver.
 *
 * @param	dv		Pointer to a device.
 * @return			__DEV_OK on success, __DEV_ERROR on error, __DEV_BUSY if
 * 					bus is busy.
 *
 */
i32 __i2cDeinit(__PDEVICE dv)
{
	__PI2C_PDB pd = dv->dv_pdb;

	__i2cCheckBusy();

	if (pd->pd_rxbuf) __heapFree(pd->pd_rxbuf);
	if (pd->pd_txbuf) __heapFree(pd->pd_txbuf);

	return __DEV_OK;
}

/*!
 * @brief Device Input/Output control function.
 *
 * Called from __deviceIOCtl().
 *
 * @param	dv			Pointer to a device.
 * @param 	cmd			Command code to execute. Can be any of the following values:
 *
 * @arg	__IOCTL_SET_OWN_ADDR
 * 		Sets the I2C own address (if configured as slave, otherwise	it has no effect).
 * 		The own address value will be taken	from the \c param argument.
 *
 * @arg	__IOCTL_SET_REMOTE_ADDR
 * 		Sets the address of the remote device (if configured as	master, otherwise it
 * 		has no effect). Every read/write operation will use the address configured with
 * 		this IOCTL,	so it should be called before any call to __i2cWrite() or __i2cRead().
 * 		The address will be taken from the \c param	argument and should not consider the
 * 		RW flag, just the first	7 MSB. For example if the address is 1010000b plus a bit
 * 		for RW, the \c param argument should just be 1010000b. The driver will shift and
 * 		set a 1 or 0 when needed.
 *
 * @arg __IOCTL_SET_REMOTE_SUBADDR
 * 		Sets the remote sub-address for those remote devices that need to address internal
 * 		memory or registers. The address will be taken from the \c data argument (pointer)
 * 		while the length (in bits) will be taken from the \c param argument and has to have
 * 		a value of 8, 16 or 32. This sub-address value will be used when reading with the
 * 		__I2C_MODE_REPEAT_START flag (used in the __i2cOpen() function), otherwise it has
 * 		no effect.
 *
 * @arg	__IOCTL_SETRXTIMEOUT
 * 		Sets the timeout for reading operations (in milliseconds). The value is taken from
 * 		the \c param argument.
 *
 * @arg	__IOCTL_GETRXTIMEOUT
 * 		Gets the timeout for reading operations (in milliseconds). The value is written to
 * 		the \c data argument as a 32-bit value.
 *
 * @arg	__IOCTL_SETTXTIMEOUT
 * 		Sets the timeout for writing operations (in milliseconds). The value is taken from
 * 		the \c param argument.
 *
 * @arg	__IOCTL_GETTXTIMEOUT
 * 		Gets the timeout for writing operations (in milliseconds). The value is written to
 * 		the \c data argument as a 32-bit value.
 *
 * @arg __IOCTL_SET_MODE
 * 		Sets the operation mode. A combination of __I2C_MODE_SLAVE or __I2C_MODE_MASTER plus
 *		__I2C_MODE_REPEAT_START.
 *
 * @param	param		Input parameter.
 * @param	data		Optional data pointer.
 * @param 	len			Length of \c data.
 * @return				__DEV_OK on success, __DEV_ERROR on error, __DEV_UNK_IOCTL if the IOCTL
 * 						code is not supported, __DEV_BUSY if the bus is busy,
 *
 */
i32 __i2cIOCtl(__PDEVICE dv, u32 cmd, u32 param, __PVOID data, u32 len)
{
	__I2C_PDB* pd = dv->dv_pdb;
	u8* ptr = (u8*) data;

	__i2cCheckBusy();

	switch (cmd)
	{
		case __IOCTL_SET_OWN_ADDR:
			/* Managed by platform */
			return __i2cSetOwnAddress(param);

		case __IOCTL_SET_REMOTE_ADDR:
			pd->pd_rmtaddr = (u8) param;
			return __DEV_OK;

		case __IOCTL_SET_REMOTE_SUBADDR:
			if (param != 8 && param != 16 && param != 32) return __DEV_ERROR;
			__memCpy(&pd->pd_subaddr, data, (param/8));
			pd->pd_subaddrlen = param;
			return __DEV_OK;

		case __IOCTL_SETRXTIMEOUT:
			pd->pd_rxtmo = (u16) param;
			return __DEV_OK;

		case __IOCTL_GETRXTIMEOUT:
			*ptr = (u32) pd->pd_rxtmo;
			return __DEV_OK;

		case __IOCTL_SETTXTIMEOUT:
			pd->pd_txtmo = (u16) param;
			return __DEV_OK;

		case __IOCTL_GETTXTIMEOUT:
			*ptr = (u32) pd->pd_txtmo;
			return __DEV_OK;

		case __IOCTL_SET_MODE:
			pd->pd_mode = (u8) param;
			return __DEV_OK;
	}

	return __DEV_UNK_IOCTL;
}

/*!
 * @brief I2C device driver open function.
 *
 * Called from __deviceOpen() to open the I2C device.
 *
 * @param	dv			Pointer to a I2C device.
 * @param 	mode		Open modes. Can be combination of any of these values:
 *
 * @arg	__I2C_MODE_SLAVE
 * 		Configure the I2C driver to act as a slave device. Cannot be used in
 * 		combination with __I2C_MODE_MASTER.
 *
 * @arg	__I2C_MODE_MASTER
 * 		Configure the I2C driver to act as a master device.	Cannot be used in
 * 		combination with __I2C_MODE_SLAVE.
 *
 * @arg	__I2C_MODE_REPEAT_START
 * 		Use in combination with the __I2C_MODE_MASTER when the remote device
 * 		needs a second START condition when reading (i.e. to send a sub-address).
 *
 * @return	__DEV_OK on success, otherwise __DEV_ERROR.
 *
 */
i32 __i2cOpen(__PDEVICE dv, u32 mode)
{
	__I2C_PDB* pd = dv->dv_pdb;
	pd->pd_mode = mode;

	/*	Init platform hardware */
	(dv->dv_plat_ioctl)(dv,	__I2C_PLAT_INIT_HW,
						0, __NULL, 0, __NULL, 0);

	/* Set TX/RX interrupt vector */
	(dv->dv_plat_ioctl)(dv,	__I2C_PLAT_SET_IRQ,
						0, __NULL, 0, __NULL, 0);

	if (!(pd->pd_mode & __I2C_MODE_MASTER))
	{
		/* Enable device if slave */
		__i2cEnable();
	}

	return __DEV_OK;
}

/*!
 * @brief I2C device driver close function.
 *
 * Called from __deviceClose() to close the I2C device.
 *
 * @param	dv			Pointer to a device.
 * @return	__DEV_OK on success, otherwise __DEV_ERROR.
 *
 */
i32 __i2cClose(__PDEVICE dv)
{
	/* Abort any pending transaction */
	__i2cAbort();

	/*	De-init hardware */
	(dv->dv_plat_ioctl)(dv,	__I2C_PLAT_DEINIT_HW,
							0, __NULL, 0, __NULL, 0);

	/* Disable TX/RX interrupt vector */
	(dv->dv_plat_ioctl)(dv,	__I2C_PLAT_RESET_IRQ,
							0, __NULL, 0, __NULL, 0);

	return __DEV_OK;
}

/*!
 * @brief Returns the count of the I2C unsent/unread bytes.
 *
 * Called from __deviceSize().
 *
 * @param	dv				Pointer to a device.
 * @param 	mode			Parameter defining on which buffer to operate.
 * @arg		__DEV_RXSIZE	Get the RX buffer unread bytes.
 * @arg		__DEV_TXSIZE	Get the TX buffer unsent bytes.
 * @return					the unsent/unread bytes quantity, __DEV_ERROR on error.
 *
 */
i32 __i2cSize(__PDEVICE dv, u8 mode)
{
	__PI2C_PDB	pd = dv->dv_pdb;

	if (mode == __DEV_RXSIZE) return(pd->pd_rxcnt);
	if (mode == __DEV_TXSIZE) return(pd->pd_txcnt);
	return __DEV_ERROR;

}


/*!
 * @brief I2C device driver flush function.
 *
 * Flush remaining unsent bytes through the I2C bus. Called from __deviceFlush().
 * Used only if the driver is configured as a master device.
 *
 * @param	dv	Pointer to a device.
 * @return	__DEV_OK on success, __DEV_TIMEOUT on timeout, __DEV_ERROR on error,
 * 			__DEV_BUSY if the bus is busy.
 *
 */
i32 __i2cFlush(__PDEVICE dv)
{
	i32 ret = __DEV_OK;
	__I2C_PDB* pd = dv->dv_pdb;

	__i2cCheckBusy();

	/* Flush works only when configured as master */
	if (!(pd->pd_mode & __I2C_MODE_MASTER) || !pd->pd_txcnt) return __DEV_ERROR;

	__i2cSetBusy(dv, __TRUE);

	/* Say the platform we're starting TX, enable device if needed */
	__i2cEnable();

	/* When using IRQs the platform should take care of the transmission */
	if (__i2cUsingIrq() != __DEV_OK)
	{
		/*
		 * Polling mode
		 */

		/* Generate START */
		if (__i2cGenerateStart() != __DEV_OK)
		{
			ret = __DEV_ERROR;
			goto ERROR;
		}

		/* Wait for START sent event */
		if (__i2cWaitForEvent(__I2C_EVENT_MASTER_MODE) != __DEV_OK)
		{
			ret = __DEV_TIMEOUT;
			goto ERROR;
		}

		/* Send address set for writing */
		if (__i2cSendAddress(pd->pd_rmtaddr << 1) != __DEV_OK)
		{
			ret = __DEV_ERROR;
			goto ERROR;
		}

		/* Wait for ACK */
		if (__i2cWaitForEvent(__I2C_EVENT_MASTER_TX) != __DEV_OK)
		{
			ret = __DEV_ERROR;
			goto ERROR;
		}

		/* Transmit buffer */
		while (pd->pd_txcnt)
		{
			if (__i2cSendByte(*(pd->pd_txbuf + pd->pd_tcidx)) != __DEV_OK)
			{
				ret = __DEV_ERROR;
				goto ERROR;
			}

			if (__i2cWaitForEvent(__I2C_EVENT_BYTE_TRANSMITTED) != __DEV_OK)
			{
				ret = __DEV_TIMEOUT;
				goto ERROR;
			}

			if (++pd->pd_tcidx >= pd->pd_txlen) pd->pd_tcidx = 0;
			--pd->pd_txcnt;
		}

		/* Generate STOP */
		if (__i2cGenerateStop() != __DEV_OK)
		{
			ret = __DEV_ERROR;
			goto ERROR;
		}

	} else {

		/*
		 * Using interrupts
		 */

		/* Reset TX event */
		__eventReset(dv->dv_txev);

		/* Start TX */
		__i2cInitTx();

		/* Wait for transmission to end */
		__eventWait(dv->dv_txev, pd->pd_txtmo);

		/* Say the platform we're finishing TX */
		__i2cEndTx();
	}


	__i2cDisable();
	__i2cSetBusy(dv, __FALSE);
	return __DEV_OK;

ERROR:

	/* Give the platform the chance to abort transmission */
	__i2cAbort();
	__i2cSetBusy(dv, __FALSE);
	return ret;
}

/*!
 * @brief I2C device driver read function.
 *
 * Call this function to read from the I2C bus. If the driver is configured as
 * a master, the reading will take place immediately. Otherwise it will poll
 * on the I2C bus until data is received or the configured timeout value is
 * reached.
 *
 * @param	dv			Pointer to a device.
 * @param 	buf			Pointer to a buffer to receive the data.
 * @param	qty			Required quantity to read.
 * @return				The quantity of bytes read, __DEV_ERROR on error, __DEV_TIMEOUT on timeout.
 *
 */
i32 __i2cRead(__PDEVICE dv, __PVOID buf, u16 qty)
{
	i32 ret = __DEV_OK;
	__I2C_PDB* pd = dv->dv_pdb;
	u8* data = (u8*) buf;
	u16 count = 0;

	__i2cCheckBusy();

	if (!qty || qty > pd->pd_rxlen) return __DEV_ERROR;

	if (pd->pd_mode & __I2C_MODE_MASTER)
	{
		/*
		 * Master receiver
		 */

		/* Enable device */
		__i2cEnable();

		/* Using interrupts? */
		if (__i2cUsingIrq() != __DEV_OK)
		{
			/*
			 * Master polling mode
			 */

			/* Generate START */
			if (__i2cGenerateStart() != __DEV_OK)
			{
				ret = __DEV_ERROR;
				goto MASTER_ERROR;
			}

			/* Wait for START sent event */
			if (__i2cWaitForEvent(__I2C_EVENT_MASTER_MODE) != __DEV_OK)
			{
				ret = __DEV_TIMEOUT;
				goto MASTER_ERROR;
			}

			if (qty == 1)
			{
				/* qty = 1 so disable ACKs */
				__i2cSetAck(__FALSE);
			} else {
				__i2cSetAck(__TRUE);
			}

			if (pd->pd_mode & __I2C_MODE_REPEAT_START)
			{
				/* Check for right sub-address length */
				if (pd->pd_subaddrlen != 8 && pd->pd_subaddrlen != 16 && pd->pd_subaddrlen != 32)
				{
					ret = __DEV_ERROR;
					goto MASTER_ERROR;
				}

				/* Send address set for writing */
				if (__i2cSendAddress(pd->pd_rmtaddr << 1) != __DEV_OK)
				{
					ret = __DEV_ERROR;
					goto MASTER_ERROR;
				}

				/* Wait for address ACK and master write mode */
				if (__i2cWaitForEvent(__I2C_EVENT_MASTER_TX) != __DEV_OK)
				{
					ret = __DEV_ERROR;
					goto MASTER_ERROR;
				}

				/* Send internal read address */
				count = pd->pd_subaddrlen;
				while (count)
				{
					if (__i2cSendByte((pd->pd_subaddr >> (count - 8)) & 0xFF) != __DEV_OK)
					{
						ret = __DEV_ERROR;
						goto MASTER_ERROR;
					}

					count -= 8;

					/* Wait for byte to be transmitted */
					if (__i2cWaitForEvent(__I2C_EVENT_BYTE_TRANSMITTED) != __DEV_OK)
					{
						ret = __DEV_TIMEOUT;
						goto MASTER_ERROR;
					}
				}

				/* Generate a START condition a second time (__I2C_MODE_REPEAT_START flag) */
				if (__i2cGenerateStart() != __DEV_OK)
				{
					ret = __DEV_ERROR;
					goto MASTER_ERROR;
				}

				/* Wait for START sent event */
				if (__i2cWaitForEvent(__I2C_EVENT_MASTER_MODE) != __DEV_OK)
				{
					ret = __DEV_TIMEOUT;
					goto MASTER_ERROR;
				}

				/* Resume normal reading */
			}

			/* Send address set for reading */
			if (__i2cSendAddress((pd->pd_rmtaddr << 1) | 1) != __DEV_OK)
			{
				ret = __DEV_ERROR;
				goto MASTER_ERROR;
			}

			/* Wait for master receiver event */
			if (__i2cWaitForEvent(__I2C_EVENT_MASTER_RX) != __DEV_OK)
			{
				ret = __DEV_TIMEOUT;
				goto MASTER_ERROR;
			}

			/* Do the reading */
			count = 0;
			while (qty)
			{
				/* Wait to receive a byte */
				if (__i2cWaitForEvent(__I2C_EVENT_BYTE_RECEIVED) == __DEV_OK)
				{
					if (qty == 2)
					{
						/* One byte left, disable ACKs */
						__i2cSetAck(__FALSE);
					}

					if (qty == 1)
					{
						/* Finishing RX, generate STOP */
						if (__i2cGenerateStop() != __DEV_OK)
						{
							ret = __DEV_ERROR;
							goto MASTER_ERROR;
						}
					}

					/* Read the received byte */
					*data =__i2cReadByte();
					data++;
					count++;
					qty--;
				}
			}

			return count;
		} else {

			/*
			 * Master receive with interrupts
			 */

			/* Reset RX event */
			__eventReset(dv->dv_rxev);

			/* Specify how many bytes are we reading in the pd_rxqty value */
			pd->pd_rxqty = qty;

			/* Say the platform to start RX */
			__i2cInitRx();

			/* Wait for transmission to end */
			if (__eventWait(dv->dv_rxev, pd->pd_rxtmo) != __EVRET_SUCCESS)
			{
				ret = __DEV_TIMEOUT;
				goto MASTER_ERROR;
			}

			/* Say the platform to end RX */
			__i2cEndRx();

			/* Reset quantity */
			pd->pd_rxqty = 0;

			/* Copy back what we have from reading */
			count = 0;

			while (count < qty)
			{
				if (pd->pd_rxcnt > 0)
				{
					*data++ = *(pd->pd_rxbuf + pd->pd_rbidx);
					if (++pd->pd_rbidx >= pd->pd_rxlen) pd->pd_rbidx = 0;
					--pd->pd_rxcnt;
					++count;
				} else break;
			}

			pd->pd_rxcnt = 0;
			__i2cDisable();
			__i2cSetBusy(dv, __FALSE);
			return count;
		}
	} else {
		/* @TODO I2C Slave receiver */
		goto SLAVE_ERROR;
	}

MASTER_ERROR:
	/* Say the platform we're finishing RX */
	__i2cEndRx();

SLAVE_ERROR:
	__i2cDisable();
	__i2cSetBusy(dv, __FALSE);
	return ret;
}

/*!
 * @brief I2C device driver write function.
 *
 * Writes to the TX buffer. Called from __deviceWrite(). Call __i2cFlush() to
 * actually transmit the buffer.
 *
 * @param	dv			Pointer to a device.
 * @param	buf			Pointer to the buffer containing data.
 * @param	qty			Quantity of bytes to write.
 * @return				Bytes written or -1 on error.
 *
 */
i32 __i2cWrite(__PDEVICE dv, __CONST __PVOID buf, u16 qty)
{
	u16 cnt = 0;
	u8* p = (u8*) buf;
	__PI2C_PDB pd = dv->dv_pdb;

	while (cnt < qty)
	{
		if (pd->pd_txcnt < pd->pd_txlen)
		{
			*(pd->pd_txbuf + pd->pd_tbidx) = *p++;
			if (++pd->pd_tbidx >= pd->pd_txlen) pd->pd_tbidx = 0;
			++pd->pd_txcnt;
			++cnt;
		} else {
			return cnt;
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

#endif /* __CONFIG_COMPILE_I2C */

