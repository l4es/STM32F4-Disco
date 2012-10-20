/***************************************************************************
 * spi.c
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

#include "spi.h"
#include <core/inc/heap.h>
#include <common/inc/common.h>

#if __CONFIG_COMPILE_SPI

/** @defgroup SPI SPI
  * @{
  */

/** @defgroup SPI_Functions Functions
  * @{
  */

/*!
 * @brief Initialization.
 *
 * Called from __deviceInit() to initialize the SPI device driver.
 *
 * @param	dv			Pointer to a device.
 * @param 	params		Pointer to a __SPI_CONFIG structure containing the default
 * 						values to initialize the device. If left to __NULL the driver
 * 						will give the platform a chance to initialize the default values.
 *
 * @return				__DEV_OK on success, otherwise __DEV_ERROR.
 *
 */
i32 __spiInit(__PDEVICE dv, __PVOID params)
{
	__PSPI_PDB pd = dv->dv_pdb;
	__PSPI_CONFIG config = (__PSPI_CONFIG) params;

	if (config == __NULL)
	{
		/* Call platform and ask for default values */
		if (((dv->dv_plat_ioctl) (dv, __SPI_PLAT_INIT_DEFAULTS, 0, __NULL, 0, __NULL, 0)) != __DEV_OK)
			return __DEV_ERROR;

	} else {
		pd->pd_rxlen = config->rxsize;
		pd->pd_txlen = config->txsize;
	}

	if (pd->pd_rxlen < __SPI_MINRXBUFLEN) pd->pd_rxlen = __SPI_MINRXBUFLEN;
	if (pd->pd_txlen < __SPI_MINTXBUFLEN) pd->pd_txlen = __SPI_MINTXBUFLEN;

	if (dv->dv_rxev) __memSet(dv->dv_rxev, 0, sizeof(__EVENT));
	if (dv->dv_txev) __memSet(dv->dv_txev, 0, sizeof(__EVENT));

	if ((pd->pd_rxbuf = __heapAlloc(pd->pd_rxlen)) == __NULL) return __DEV_ERROR;
	if ((pd->pd_txbuf = __heapAlloc(pd->pd_txlen)) == __NULL) {
		__heapFree(pd->pd_rxbuf);
		return __DEV_ERROR;
	}

	return __DEV_OK;
}

/*!
 * @brief SPI device driver destroy.
 *
 * Called from __deviceDeinit().
 *
 * @param	dv			Pointer to a device.
 * @return				__DEV_OK on success, otherwise __DEV_ERROR.
 *
 */
i32 __spiDeinit(__PDEVICE dv)
{
	__PSPI_PDB pd = dv->dv_pdb;

	/* Check for still opened */
	if (dv->dv_opcnt != 0) return __DEV_ERROR;

	if (pd != __NULL)
	{
		if (pd->pd_rxbuf != __NULL) __heapFree(pd->pd_rxbuf);
		if (pd->pd_txbuf != __NULL) __heapFree(pd->pd_txbuf);
	}

	return __DEV_OK;
}

/*!
 * @brief Device Input/Output control function.
 *
 * Called from __deviceIOCtl().
 *
 * @param	dv			Pointer to a device.
 * @param 	cmd			Command code to execute.
 *
 * 	@arg __IOCTL_ASSERT_CS
 * 		Enables/disables CS generation (master only).
 *
 * 	@arg __IOCTL_SET_CS
 * 		\c param value (1 or 0) sets the CS pin high/low (master only).
 *
 * 	@arg __IOCTL_SETSPEED
 * 		\c param value sets the communication speed.
 *
 * 	@arg __IOCTL_SETRXTIMEOUT
 * 		\c param value for the transmission timeout.
 *
 * 	@arg __IOCTL_SETTXTIMEOUT
 * 		\c param value for the reception timeout.
 *
 * @param	param		Input parameter.
 * @param	data		Optional data pointer.
 * @param	len			Length of \c data.
 * @return				__DEV_OK on success, otherwise __DEV_ERROR or __DEV_UNK_IOCTL.
 *
 */
i32 __spiIOCtl(__PDEVICE dv, u32 cmd, u32 param, __PVOID data, u32 len)
{
	__PSPI_PDB	pd = dv->dv_pdb;

	switch(cmd)
	{
		case __IOCTL_ASSERT_CS:
			pd->pd_asscs = (u8) param;
			break;

		case __IOCTL_SET_CS:
			(dv->dv_plat_ioctl)(dv, __SPI_PLAT_SET_CS, param,
								__NULL, 0, __NULL, 0);
			break;

		case __IOCTL_SETSPEED:
			(dv->dv_plat_ioctl)(dv, __SPI_PLAT_SET_SPEED, param,
								__NULL, 0, __NULL, 0);
			break;

		case __IOCTL_SETRXTIMEOUT:
			pd->pd_rxtmo = param;
			break;

		case __IOCTL_SETTXTIMEOUT:
			pd->pd_txtmo = param;
			break;
		default:
			return __DEV_UNK_IOCTL;
	}

	return __DEV_OK;
}

/*!
 * @brief SPI device driver open function.
 *
 * Called from __deviceOpen() to open the SPI device.
 *
 * @param	dv			Pointer to a device.
 * @param 	mode		Open mode. Can be one of the following values:
 *
 * @arg __SPIMODE_MASTER 	Master mode.
 * @arg __SPIMODE_SLAVE 	Slave mode.
 *
 * @return				__DEV_OK on success, otherwise __DEV_ERROR.
 *
 */
i32 __spiOpen(__PDEVICE dv, u32 mode)
{

	__PSPI_PDB pd = dv->dv_pdb;

	pd->pd_mode = mode;

	/*	Init hardware */
	(dv->dv_plat_ioctl)(dv,	__SPI_PLAT_INIT_HW,
						0, __NULL, 0, __NULL, 0);

	/* Set TX/RX interrupt vector */
	(dv->dv_plat_ioctl)(dv,	__SPI_PLAT_SET_IRQ,
						0, __NULL, 0, __NULL, 0);

	return __DEV_OK;
}

/*!
 * @brief SPI device driver close function.
 *
 * Called from __deviceClose() to close the SPI device.
 *
 * @param	dv			Pointer to a device.
 * @return				__DEV_OK on success, otherwise __DEV_ERROR.
 *
 */
i32 __spiClose(__PDEVICE dv)
{

	/*	De-init hardware */
	(dv->dv_plat_ioctl)(dv,	__SPI_PLAT_DEINIT_HW,
						0, __NULL, 0, __NULL, 0);

	/* Disable TX/RX interrupt vector */
	(dv->dv_plat_ioctl)(dv,	__SPI_PLAT_RESET_IRQ,
						0, __NULL, 0, __NULL, 0);

	return __DEV_OK;
}

/*!
 * @brief Returns the count of the SPI unsent/unread bytes.
 *
 * Called from __deviceSize().
 *
 * @param	dv				Pointer to a device.
 * @param 	mode			Parameter defining on which buffer operate.
 * @arg		__DEV_RXSIZE	Get the RX buffer unread bytes.
 * @arg		__DEV_TXSIZE	Get the TX buffer unsent bytes.
 * @return					the SPI unsent/unread bytes. -1 on error.
 *
 */
i32 __spiSize(__PDEVICE dv, u8 mode)
{
	__PSPI_PDB pd = dv->dv_pdb;

	if (mode == __DEV_RXSIZE) return(pd->pd_rxcnt);
	if (mode == __DEV_TXSIZE) return(pd->pd_txcnt);

	return __DEV_OK;
}

/*!
 * @brief SPI device driver flush function.
 *
 * Flush remaining unsent bytes. Called from __deviceFlush().
 *
 * @param	dv		Pointer to a device.
 * @return			__DEV_OK on success, __DEV_ERROR on error, __DEV_TIMEOUT on timeout.
 *
 */
i32 __spiFlush(__PDEVICE dv)
{
	__PSPI_PDB pd = dv->dv_pdb;
	i32 ret = 0;

	/* something to send? */
	if (!pd->pd_txcnt) return __DEV_ERROR;

	if (dv->dv_txev) __eventReset(dv->dv_txev);

	/*	Init transmission */
	(dv->dv_plat_ioctl)(dv,	__SPI_PLAT_INIT_TX,  pd->pd_asscs,
						__NULL, 0, __NULL, 0);

	if (dv->dv_txev) {
		if (__eventWait(dv->dv_txev, pd->pd_txtmo) == __EVRET_SUCCESS) {
			ret = __DEV_OK;
		} else {
			ret = __DEV_TIMEOUT;
		}
	}

	/* End transmission */
	(dv->dv_plat_ioctl)(dv,	__SPI_PLAT_END_TX,  pd->pd_asscs,
						__NULL, 0, __NULL, 0);

	return ret;
}



/*!
 * @brief SPI device driver read function.
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
i32 __spiRead(__PDEVICE dv, __PVOID buf, u16 qty)
{
	u16	cnt = 0;
	__PSTRING p = buf;
	__PSPI_PDB pd = dv->dv_pdb;

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
			/* Event provided, reset it */
			if (!dv->dv_rxev) return cnt;
			__eventReset(dv->dv_rxev);

			if (__eventWait(dv->dv_rxev,(u32) pd->pd_rxtmo) == __EVRET_TIMEOUT) {
				return cnt;
			}
		}
	}
	return cnt;
}

/*!
 * @brief SPI device driver write function.
 *
 * Writes to the TX buffer. Called from __deviceWrite().
 *
 * @param	dv			Pointer to a device.
 * @param	buf			Pointer to the buffer containing data.
 * @param	qty			Quantity of bytes to write.
 * @return				Bytes written or -1 on error.
 *
 */
i32 __spiWrite(__PDEVICE dv, __CONST __PVOID buf, u16 qty)
{
	u16 cnt = 0;
	u8* p = (u8*) buf;
	__PSPI_PDB pd = dv->dv_pdb;

	while (cnt < qty)
	{
		if (pd->pd_txcnt < pd->pd_txlen)
		{
			*(pd->pd_txbuf + pd->pd_tbidx) = *p++;
			if (++pd->pd_tbidx >= pd->pd_txlen) pd->pd_tbidx = 0;
			++pd->pd_txcnt;
			++cnt;
		} else {
			if (__spiFlush(dv) != __DEV_OK) {
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

#endif /* __CONFIG_COMPILE_SPI */
