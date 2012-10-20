/***************************************************************************
 * plat_spi.c
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

/*
 * This file is part of the spi.c driver
 * for the stm32. Bounds the spi.c interface
 * with the hardware.
 */

#include "plat_spi.h"
#include <drivers/inc/spi.h>
#include <core/inc/intrvect.h>

#if __CONFIG_COMPILE_SPI

/*
 *
 */
__STATIC u8 __spiSetParameters(__PDEVICE dv)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	PSPI_PARAMS params;
	__PSPI_PDB pd;
	SPI_TypeDef* pSPI;
	u8 pin_source;

	params = dv->dv_params;
	pSPI = params->base_addr;
	pd = dv->dv_pdb;

	if (params->base_addr == SPI1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	} else if (params->base_addr == SPI2)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	} else if (params->base_addr == SPI3)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	}

	/* Select alternative function for IO pins */
	pin_source	= 0x00;
	while (params->pin_clk > (1<<pin_source)){
		pin_source++;
	}
	if ((params->base_addr == SPI1)||(params->base_addr == SPI2)){
		GPIO_PinAFConfig(params->port_clk, pin_source, 0x05);	// GPIO_AF_SPI1 = GPIO_AF_SPI2 = 0x05
	}else{
		GPIO_PinAFConfig(params->port_clk, pin_source, 0x06);	// GPIO_AF_SPI3 = 0x06
	}

	pin_source	= 0x00;
	while (params->pin_miso > (1<<pin_source)){
		pin_source++;
	}
	if ((params->base_addr == SPI1)||(params->base_addr == SPI2)){
		GPIO_PinAFConfig(params->port_miso, pin_source, 0x05);	// GPIO_AF_SPI1 = GPIO_AF_SPI2 = 0x05
	}else{
		GPIO_PinAFConfig(params->port_miso, pin_source, 0x06);	// GPIO_AF_SPI3 = 0x06
	}

	pin_source	= 0x00;
	while (params->pin_mosi > (1<<pin_source)){
		pin_source++;
	}
	if ((params->base_addr == SPI1)||(params->base_addr == SPI2)){
		GPIO_PinAFConfig(params->port_mosi, pin_source, 0x05);	// GPIO_AF_SPI1 = GPIO_AF_SPI2 = 0x05
	}else{
		GPIO_PinAFConfig(params->port_mosi, pin_source, 0x06);	// GPIO_AF_SPI3 = 0x06
	}

	/* 1 bit for pre-emption priority, 3 bits for subpriority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	/* Master or slave? */
	if (pd->pd_mode == __SPIMODE_MASTER)
	{
		/* Master MOSI and CLK as alternate function */
		__cpuPinConfigure(params->pin_mosi, params->port_mosi, GPIO_Speed_50MHz, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL);
		__cpuPinConfigure(params->pin_clk, params->port_clk, GPIO_Speed_50MHz, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL);

		/* MISO as input with pull-up */
		__cpuPinConfigure(params->pin_miso, params->port_miso, GPIO_Speed_50MHz, GPIO_Mode_IN,  GPIO_OType_PP, GPIO_PuPd_UP);

		/* CS as output */
		if (params->pin_cs && params->port_cs)
		{
			__cpuPinConfigure(params->pin_cs, params->port_cs, GPIO_Speed_50MHz, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);
		}		
	} else
	{
		/* @TODO implement SPI slave */
	}

	/* Configure and enable interrupt -------------------------------*/
	NVIC_InitStructure.NVIC_IRQChannel = dv->dv_txint;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

  	SPI_InitStructure.SPI_Direction = params->direction;
  	SPI_InitStructure.SPI_Mode = (pd->pd_mode == __SPIMODE_MASTER) ? SPI_Mode_Master : SPI_Mode_Slave;
  	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  	SPI_InitStructure.SPI_CPOL = params->cpol;
  	SPI_InitStructure.SPI_CPHA = params->cpha;
	SPI_InitStructure.SPI_NSS = params->nss_mode;	
  	SPI_InitStructure.SPI_BaudRatePrescaler = params->prescaler;
  	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  	SPI_InitStructure.SPI_CRCPolynomial = 7;
  	SPI_Init(pSPI, &SPI_InitStructure);

 	SPI_Cmd(pSPI, ENABLE);

 	return __DEV_OK;
}

__STATIC __BOOL __spiCharOutput(__PDEVICE dv, u8 c)
{
	PSPI_PARAMS params = dv->dv_params;

	if (SPI_I2S_GetFlagStatus(params->base_addr, SPI_I2S_FLAG_BSY) == RESET)
	{
		SPI_I2S_SendData(params->base_addr, c);
		return __TRUE;
	}

	return __FALSE;
}

__VOID __spiIsr(__PVOID param)
{
	__PDEVICE dv;
	PSPI_PARAMS params;
	SPI_TypeDef* pSPI;
	__PSPI_PDB pd;

	dv = (__PDEVICE) param;
	pd = dv->dv_pdb;
	params = dv->dv_params;
	pSPI = params->base_addr;
	
	/* check rx buff not empty */
	if (SPI_I2S_GetITStatus(pSPI, SPI_I2S_IT_RXNE) != RESET)
	{
		*(pd->pd_rxbuf + pd->pd_rcidx) = SPI_I2S_ReceiveData(pSPI);
		if (++pd->pd_rcidx >= pd->pd_rxlen) pd->pd_rcidx = 0;
		if (++pd->pd_rxcnt > pd->pd_rxlen) pd->pd_rxerr |= __SPIERR_OVERFLOW;

		if (dv->dv_rxev) __eventSet(dv->dv_rxev);

		if (pd->pd_asscs && !pd->pd_txcnt && params->pin_cs && (pd->pd_mode == __SPIMODE_MASTER) &&
			SPI_I2S_GetFlagStatus(pSPI, SPI_I2S_FLAG_BSY) == RESET)
		{
			__pinSet(params->port_cs, params->pin_cs, __TRUE);
		}
	}
	
	/* check tx buff empty */
	if (SPI_I2S_GetITStatus(pSPI, SPI_I2S_IT_TXE) != RESET)
	{
		if (pd->pd_txcnt)
		{
			if (__spiCharOutput(dv, *(pd->pd_txbuf + pd->pd_tcidx)))
			{
				if (++pd->pd_tcidx >= pd->pd_txlen) pd->pd_tcidx = 0;
				--pd->pd_txcnt;
			}
		} else
		{
			/* End of transmission */
			if (SPI_I2S_GetFlagStatus(pSPI, SPI_I2S_FLAG_BSY) == RESET) {
				SPI_I2S_ITConfig(pSPI, SPI_I2S_IT_TXE, DISABLE);
				if (dv->dv_txev) __eventSet(dv->dv_txev);
			}
		}
	}
}

__STATIC __VOID __spiEnableIrq(__PDEVICE dv)
{
	PSPI_PARAMS params;
	SPI_TypeDef* pSPI;

	params = dv->dv_params;
	pSPI = params->base_addr;

	__intSetVector(dv->dv_txint,__spiIsr,dv);
	SPI_I2S_ITConfig(pSPI, SPI_I2S_IT_RXNE, ENABLE);
}

__STATIC __VOID __spiDisableIrq(__PDEVICE dv)
{
	PSPI_PARAMS params;
	SPI_TypeDef* pSPI;

	params = dv->dv_params;
	pSPI = params->base_addr;

	__intSetVector(dv->dv_txint,__NULL,__NULL);
	SPI_I2S_ITConfig(pSPI, SPI_I2S_IT_RXNE, DISABLE);
}

__STATIC u8 __spiInitTx(__PDEVICE dv)
{
	PSPI_PARAMS params = dv->dv_params;
	SPI_I2S_ITConfig(params->base_addr, SPI_I2S_IT_TXE, ENABLE);
	return __DEV_OK;
}

__STATIC __VOID __spiSpeed(__PDEVICE dv, u8 speed )
{
	PSPI_PARAMS params;
	SPI_TypeDef* pSPI;
	u32 tmp;

	params = dv->dv_params;
	pSPI = params->base_addr;

	tmp = pSPI->CR1;
	if ( speed == 0 )
	{
		/* Set slow clock (100k-400k) */
		tmp = ( tmp | SPI_BaudRatePrescaler_256 );
	} else
	{
		/* Set fast clock (depends on the CSD) */
		tmp = ( tmp & ~SPI_BaudRatePrescaler_256 ) | SPI_BaudRatePrescaler_4;
	}
	pSPI->CR1 = tmp;
}

/*!
 * @brief SPI for stm32 IO control function
 *
 * Called from @ref SPI driver to perform platform-related
 * tasks, like pin configuration or baudrate settings.
 *
 * @param	dv		Pointer to device.
 * @param	code	IO control code.
 *
 * @arg	__SPI_PLAT_INIT_HW			Initialize hardware.
 * @arg __SPI_PLAT_SET_BAUDRATE		Set baudrate.
 * @arg	__SPI_PLAT_CHAR_OUTPUT		Character output.
 * @arg	__SPI_PLAT_CHAR_INPUT		Character input.
 * @arg __SPI_PLAT_INIT_TX			Start transmission. If \c param = 1, CS is asserted.
 * @arg __SPI_PLAT_SET_IRQ			Configure interrupts.
 *
 * @param	param	Optional parameter.
 * @param	in		Input buffer pointer.
 * @param	in_len	Input buffer pointer length.
 * @param	out		Output buffer pointer.
 * @param	out_len Output buffer pointer length.
 *
 * @return 	A value depending on the requested code execution.
 *
 */
i32 __spiPlatIoCtl(__PDEVICE dv, u32 code, u32 param, __PVOID in, u32 in_len, __PVOID out, u32 out_len)
{
	PSPI_PARAMS params = dv->dv_params;
	__PSPI_PDB pd = dv->dv_pdb;

	switch (code)
	{
		case __SPI_PLAT_INIT_HW:
			return __spiSetParameters(dv);

		case __SPI_PLAT_SET_SPEED:
			// TODO move it to SD driver
			__spiSpeed(dv, param);
			break;

		case __SPI_PLAT_INIT_TX:
			if (params->pin_cs && (pd->pd_mode & __SPIMODE_MASTER) && param)
			{
				__pinSet(params->port_cs, params->pin_cs, __FALSE);
			}
			return __spiInitTx(dv);

		case __SPI_PLAT_SET_IRQ:
			__spiEnableIrq(dv);
			break;

		case __SPI_PLAT_SET_CS:
			if (params->pin_cs) __pinSet(params->port_cs, params->pin_cs, (u8) param);
			break;
			
		case __SPI_PLAT_END_TX:
			break;

		case __SPI_PLAT_RESET_IRQ:
			__spiDisableIrq(dv);
			break;

		case __SPI_PLAT_DEINIT_HW:
			break;

		case __SPI_PLAT_INIT_DEFAULTS:
			/* Use default values */
			pd->pd_rxlen = __PLATSPI_RXBUFLEN;
			pd->pd_txlen = __PLATSPI_TXBUFLEN;
			return __DEV_OK;

		default:
			return __DEV_UNK_IOCTL;
	}

	return __DEV_OK;
}

#endif /* __CONFIG_COMPILE_SPI */

