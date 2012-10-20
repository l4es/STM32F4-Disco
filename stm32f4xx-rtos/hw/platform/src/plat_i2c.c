/***************************************************************************
 * plat_i2c.c
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

#include "plat_i2c.h"
#include <drivers/inc/i2c.h>
#include <core/inc/intrvect.h>

#if __CONFIG_COMPILE_I2C

/** @addtogroup I2C_Stm32 STM32
  * @{
  */

/** @addtogroup I2C_Stm32_Constants Constants
  * @{
  */

/** @defgroup I2C_Stm32_PrivateConstants Private constants
  * @{
  */

/*!
 * @brief Reception state codes
 *
 * ST has done this I2C interface a little tricky,
 * so we will be using this state codes for reception.
 */
#define I2C_RXSTATE_RESET			0	/*!< @brief Initial (reset) state */
#define I2C_RXSTATE_SEND_SUBADDR	1	/*!< @brief Send sub-address */
#define I2C_RXSTATE_SUBADDR_SENT	2	/*!< @brief Sub-address sent */
#define I2C_RXSTATE_1				3	/*!< @brief DataN-2 */
#define I2C_RXSTATE_2				4	/*!< @brief DataN-2 and DataN-1 */
#define I2C_RXSTATE_3				5	/*!< @brief DataN */
#define I2C_RXSTATE_4				6	/*!< @brief DataN */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup I2C_Stm32_Functions Functions
  * @{
  */

/*!
 * @brief Sets hardware parameters.
 *
 * @param	dv		Device to configure.
 *
 * @return Nothing.
 */
__STATIC u8 __i2cSetParameters(__PDEVICE dv)
{
	I2C_InitTypeDef I2C_InitStructure;
	PI2C_PARAMS params;
	u8 pin_source;

	params = dv->dv_params;

	I2C_DeInit(params->base_addr);
	I2C_Cmd(params->base_addr, DISABLE);

	I2C_SoftwareResetCmd(params->base_addr, ENABLE);
	I2C_SoftwareResetCmd(params->base_addr, DISABLE);

	/* Configure SDA and SCL as alternate output */
	__cpuPinConfigure(params->scl_pin | params->sda_pin, params->port_addr,  GPIO_Speed_50MHz, GPIO_Mode_AF, GPIO_OType_OD, GPIO_PuPd_NOPULL);

	/* Select alternative function for IO pins */
	pin_source	= 0x00;
	while (params->scl_pin > (1<<pin_source)){
		pin_source++;
	}
	GPIO_PinAFConfig(params->port_addr, pin_source, 0x04); //GPIO_AF_I2C1 = GPIO_AF_I2C2 = GPIO_AF_I2C3 = 0x04

	pin_source	= 0x00;
	while (params->sda_pin > (1<<pin_source)){
		pin_source++;
	}
	GPIO_PinAFConfig(params->port_addr, pin_source, 0x04); //GPIO_AF_I2C1 = GPIO_AF_I2C2 = GPIO_AF_I2C3 = 0x04

	/* Enable APB clock */
	RCC_APB1PeriphClockCmd(params->bus_clock_addr, ENABLE);

	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = params->default_speed;

	I2C_Init(params->base_addr, &I2C_InitStructure);
	I2C_Cmd(params->base_addr, ENABLE);

	return __DEV_OK;
}

/*!
 * @brief Sets I2C bus speed.
 *
 * @param dv	Device to configure.
 * @param spd	Speed in Hz.
 *
 * @return __DEV_OK on success, otherwise an error code.
 *
 */
__STATIC u8 __i2cSetSpeed(__PDEVICE dv, u32 spd)
{
	PI2C_PARAMS params = dv->dv_params;
	I2C_InitTypeDef I2C_InitStructure;

	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = spd;
	I2C_Init(params->base_addr, &I2C_InitStructure);

	params->default_speed = spd;

  	return __DEV_OK;
}

#if __PLATI2C_MODE_INTERRUPTS

/*!
 * @brief Transmits a byte from the driver's TX buffer, and increment
 * the offset variables.
 *
 * @param pd		PDB of the I2C device.
 * @param params	Pointer to a I2C_PARAMS structure.
 *
 * @return			__TRUE if there is still data to send. __FALSE if there is no
 * 					more data to send.
 *
 */
__STATIC __BOOL __i2cTxBufferByte(__I2C_PDB* pd, I2C_PARAMS* params)
{
	if (pd->pd_txcnt)
	{
		I2C_SendData(params->base_addr, *(pd->pd_txbuf + pd->pd_tcidx));
		if (++pd->pd_tcidx >= pd->pd_txlen) pd->pd_tcidx = 0;
		if (--pd->pd_txcnt == 0) return __FALSE;

		return __TRUE;
	}

	return __FALSE;
}


/*!
 * @brief Receives a byte from the I2C and stores it in the RX buffer
 * of the driver.
 *
 * @param pd		PDB of the I2C device.
 * @param params	Pointer to a I2C_PARAMS structure.
 *
 * @return			Nothing.
 *
 */
__STATIC __VOID __i2cRxBufferByte(__I2C_PDB* pd, I2C_PARAMS* params)
{
	/* Receive byte */
	*(pd->pd_rxbuf + pd->pd_rcidx) = I2C_ReceiveData(params->base_addr);
	if (++pd->pd_rcidx >= pd->pd_rxlen) pd->pd_rcidx = 0;
	if (++pd->pd_rxcnt > pd->pd_rxlen) pd->pd_rxerr |= __I2C_ERR_OVERFLOW;
}

/*!
 * @brief Main interrupt service routine for the I2C driver.
 */
__VOID __i2cISR(__PVOID pVoid)
{
	__PDEVICE dv = (__PDEVICE) pVoid;
	__I2C_PDB* pd = dv->dv_pdb;
	I2C_PARAMS* params = dv->dv_params;

	__systemEnterISR();

	/* Check the current event using the ST library */
	switch (I2C_GetLastEvent(params->base_addr))
	{
		/*
		 * Received after sending the START bit (EV 5)
		 */
		case I2C_EVENT_MASTER_MODE_SELECT:

			/* Check if there is something to read */
			if (pd->pd_rxqty)
			{
				/*
				 * READING
				 */

				/* Has to send a second START? */
				if (pd->pd_mode & __I2C_MODE_REPEAT_START && params->rxstep == I2C_RXSTATE_RESET)
				{
					/*
					 * Starting sub-address transmission.
					 */

					/* Send address in write mode */
					params->rxstep = I2C_RXSTATE_SEND_SUBADDR;
					I2C_Send7bitAddress(params->base_addr, (u8) (pd->pd_rmtaddr << 1), I2C_Direction_Transmitter);
				} else {

					/*
					 * Prepare reception code for I2C_EVENT_MASTER_BYTE_RECEIVED event, to struggle
					 * with the N>2, N=2, N=1 situation.
					 */
					params->rxstep = I2C_RXSTATE_1;

					/* Send address in read mode */
					I2C_Send7bitAddress(params->base_addr, (u8) (pd->pd_rmtaddr << 1) | 1, I2C_Direction_Receiver);
				}
			} else {
				/*
				 * WRITING
				 */

				/* Send address in write mode */
				I2C_Send7bitAddress(params->base_addr, (u8) (pd->pd_rmtaddr << 1), I2C_Direction_Transmitter);
			}
			break;

		/*
		 * After sending the address and receiving the ACK (EV 6)
		 */
		case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:

			/* If reading send the first byte of the sub-address */
			if (pd->pd_rxqty && params->rxstep == I2C_RXSTATE_SEND_SUBADDR)
			{
				params->subaddr_cnt = pd->pd_subaddrlen;
				I2C_SendData(params->base_addr, (u8) (pd->pd_subaddr >> (params->subaddr_cnt - 8)) & 0xFF);
				params->subaddr_cnt -= 8;

				if (params->subaddr_cnt == 0)
				{
					I2C_ITConfig(params->base_addr, I2C_IT_BUF, DISABLE);
				}

			} else {
				/* If writing start sending bytes */
				if (__i2cTxBufferByte(pd, params) == __FALSE)
				{
					I2C_ITConfig(params->base_addr, I2C_IT_BUF, DISABLE);
				}
			}
			break;

		/*
		 * I2C TX buffer is empty and the data is being output through the shift register (EV 8)
		 */
		case I2C_EVENT_MASTER_BYTE_TRANSMITTING:

			/* If reading and sending the sub-address */
			if (pd->pd_rxqty)
			{
				if (params->rxstep == I2C_RXSTATE_SEND_SUBADDR)
				{
					if (params->subaddr_cnt)
					{
						/* Send sub-address byte */
						I2C_SendData(params->base_addr, (u8) (pd->pd_subaddr >> (params->subaddr_cnt - 8)) & 0xFF);

						/* Keep track of the sub-address byte we have to send */
						params->subaddr_cnt -= 8;

						if (params->subaddr_cnt == 0)
						{
							I2C_ITConfig(params->base_addr, I2C_IT_BUF, DISABLE);
						}

					} else {
						/* Sub-address sent, wait for I2C_EVENT_MASTER_BYTE_TRANSMITTED */
					}
				}
			} else {
				/* If transmitting, keep sending */
				if (__i2cTxBufferByte(pd, params) == __FALSE)
				{
					I2C_ITConfig(params->base_addr, I2C_IT_BUF, DISABLE);
				}
			}
			break;

		/*
		 * After transmitting a byte (EV 8_2)
		 */
		case I2C_EVENT_MASTER_BYTE_TRANSMITTED:

			/* Check if we're done with the sub-address */
			if (pd->pd_rxqty)
			{
				if (params->rxstep == I2C_RXSTATE_SEND_SUBADDR)
				{
					if (params->subaddr_cnt == 0)
					{
						/* Send START again */
						I2C_GenerateSTART(params->base_addr, ENABLE);

						/* Change step to prevent sending sub-address twice */
						params->rxstep = I2C_RXSTATE_SUBADDR_SENT;
					} else {
						/* Send sub-address byte */
						I2C_SendData(params->base_addr, (u8) (pd->pd_subaddr >> (params->subaddr_cnt - 8)) & 0xFF);

						/* Keep track of the sub-address byte we have to send */
						params->subaddr_cnt -= 8;

						if (params->subaddr_cnt == 0)
						{
							I2C_ITConfig(params->base_addr, I2C_IT_BUF, DISABLE);
						}
					}
				}
			} else {
				/* If transmitting, check if there are still more to send */
				if (!pd->pd_txcnt)
				{
					/* Done with transmission */
					I2C_GenerateSTOP(params->base_addr, ENABLE);
					I2C_ITConfig(params->base_addr, I2C_IT_EVT, DISABLE);
					__eventSet(dv->dv_txev);
				} else {
					/* Not done, we may have had a delay, keep sending */
					if (__i2cTxBufferByte(pd, params) == __FALSE)
					{
						I2C_ITConfig(params->base_addr, I2C_IT_BUF, DISABLE);
					}
				}
			}
			break;

		/*
		 * After sending the address in read-mode
		 */
		case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:

			I2C_ITConfig(params->base_addr, I2C_IT_BUF, ENABLE);

			/* Disable ACKs generation if reading just one byte */
			if (pd->pd_rxqty == 1)
			{
				I2C_AcknowledgeConfig(params->base_addr, DISABLE);
				I2C_GenerateSTOP(params->base_addr, ENABLE);
			}
			break;

		/*
		 * MASTER READING
		 */

		/* From ST documentation
		 *
		 * Method 2: transfer sequence for master receiver when N>2
		 *
		 * When 3 bytes remain to be read:
		 *	(1) RxNE = 1 => Nothing (DataN-2 not read)
		 *	(2) DataN-1 received
		 *	(3) BTF = 1 because both shift and data registers are full: DataN-2 in DR and DataN-1 in
		 *	  the shift register => SCL tied low: no other data will be received on the bus.
		 *	(4) Clear ACK bit
		 *	(5) Read DataN-2 in DR => This will launch the DataN reception in the shift register
		 *	(6) DataN received (with a NACK)
		 *	(7) Program START/STOP
		 *	(8) Read DataN-1
		 *	(9) RxNE = 1
		 *	(10) Read DataN
		 *
		 * So we will be handling special N>2 case here, reusing params->rxstep.
		 * params->rxstep is set to I2C_RXSTATE_1 in the I2C_EVENT_MASTER_MODE_SELECT
		 * event after sending the START bit, and eventually handling the second START
		 * condition.
		 *
		 * I2C_RXSTATE_1 and I2C_RXSTATE_4 steps will be handled in the
		 * I2C_EVENT_MASTER_BYTE_RECEIVED event case. I2C_RXSTATE_2 and I2C_RXSTATE_3
		 * will be handled in the (I2C_EVENT_MASTER_BYTE_RECEIVED | 0x4) case (BUSY flag
		 * plus I2C_EVENT_MASTER_BYTE_RECEIVED event).
		 *
		 */

		case I2C_EVENT_MASTER_BYTE_RECEIVED:

			if (pd->pd_rxqty > 2 && pd->pd_rxcnt >= pd->pd_rxqty - 3)
			{
				switch (params->rxstep)
				{
					case I2C_RXSTATE_1:
						/* (1) RxNE = 1 => Nothing (DataN-2 not read) */
						params->rxstep = I2C_RXSTATE_2;
						__systemLeaveISR();
						return;

					case I2C_RXSTATE_4:
						/*
						 *	(9) RxNE = 1
						 *	(10) Read DataN
						 */

						/* break here, the reading is done below */
						break;

					default:
						__systemLeaveISR();
						return;
				}
			}

			/*
			 * Handle pd->pd_rxqty == 2
			 */

			/*
			 * Handle pd->pd_rxqty == 1
			 */

			__i2cRxBufferByte(pd, params);

			if (pd->pd_rxcnt == pd->pd_rxqty)
			{
				__eventSet(dv->dv_rxev);
				I2C_ITConfig(params->base_addr, I2C_IT_BUF | I2C_IT_EVT , DISABLE);
			}
			break;

		case (I2C_EVENT_MASTER_BYTE_RECEIVED | 0x4):
			if (pd->pd_rxqty > 2 && pd->pd_rxcnt >= pd->pd_rxqty - 3)
			{
				switch (params->rxstep)
				{
					case I2C_RXSTATE_2:
						/*
						 * (2) DataN-1 received
						 * (3) BTF = 1 because both shift and data registers are full: DataN-2 in DR and DataN-1 in
						 * the shift register => SCL tied low: no other data will be received on the bus.
						 */

						/*
						 * (4) Clear ACK bit
						 */
						I2C_AcknowledgeConfig(params->base_addr, DISABLE);

						/*
						 * (5) Read DataN-2 in DR => This will launch the DataN reception in the shift register
						 */
						__i2cRxBufferByte(pd, params);
						params->rxstep = I2C_RXSTATE_3;

					case I2C_RXSTATE_3:
						/*
						 * (6) DataN received (with a NACK)
						 */

						/*
						 * (7) Program START/STOP
						 */
						 I2C_GenerateSTOP(params->base_addr, ENABLE);

						/*
						 * (8) Read DataN-1
						 */
						__i2cRxBufferByte(pd, params);

						/*
						 * Wait for DataN (the next interrupt).
						 */
						params->rxstep = I2C_RXSTATE_4;

						__systemLeaveISR();
						return;
				}
			}
			break;
	}

	__systemLeaveISR();
}

#endif /* __PLATI2C_MODE_INTERRUPTS */

/*!
 * @brief I2C for stm32 IO control function
 *
 * Called from @ref I2C driver to perform platform-related
 * tasks.
 *
 * @param	dv		Pointer to device.
 * @param	code	IO control code. One of the following values
 *
 * @arg	@ref __I2C_PLAT_INIT_HW			Initialize hardware.
 * @arg @ref __I2C_PLAT_SET_SPEED		Set bus speed.
 * @arg @ref __I2C_PLAT_INIT_TX			Start transmission.
 * @arg @ref __I2C_PLAT_SET_IRQ			Configure interrupts.
 * @arg @ref __I2C_PLAT_ENABLE_DEVICE	Enable device.
 * @arg @ref __I2C_PLAT_DISABLE_DEVICE	Disable device.
 * @arg @ref __I2C_PLAT_SET_IRQ			Set interrupts.
 * @arg @ref __I2C_PLAT_RESET_IRQ		Disable interrupts for this device.
 * @arg @ref __I2C_PLAT_SET_ADDRESS		Transmit the remote device address.
 * @arg @ref __I2C_PLAT_SET_OWN_ADDRESS	Configure our own address (slave mode).
 * @arg @ref __I2C_PLAT_GENERATE_START	Generate START condition.
 * @arg @ref __I2C_PLAT_GENERATE_STOP	Generate STOP condition.
 * @arg @ref __I2C_PLAT_CHECK_EVENT		Check for a given event.
 * @arg @ref __I2C_PLAT_SET_ACK			Configure automatic ACK generation.
 * @arg @ref __I2C_PLAT_ABORT			Abort and set to a reset (stable) state.
 * @arg @ref __I2C_PLAT_USE_IRQ			Driver is asking is the platform supports interrupt
 * 										mode.
 * @arg @ref __I2C_PLAT_INIT_TX			Start transmission.
 * @arg @ref __I2C_PLAT_END_TX			End transmission.
 * @arg @ref __I2C_PLAT_INIT_RX			Start reception.
 * @arg @ref __I2C_PLAT_END_RX			End reception.
 * @arg @ref __I2C_PLAT_WRITE_BYTE		Write a single byte.
 * @arg @ref __I2C_PLAT_READ_BYTE		Read a single byte.
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
i32 __i2cPlatIoCtl(__PDEVICE dv, u32 code, u32 param, __PVOID in, u32 in_len, __PVOID out, u32 out_len)
{
	I2C_PARAMS* params = dv->dv_params;
	I2C_InitTypeDef I2C_InitStructure;
	u32 time;

	switch (code)
	{
		/*
		 * Using IRQ mode, platform will make every transaction on its own.
		 */
		case __I2C_PLAT_USE_IRQ:
#if __PLATI2C_MODE_INTERRUPTS
			return __DEV_OK;
#else
			return __DEV_ERROR; //__DEV_OK;
#endif /* __PLATI2C_MODE_INTERRUPTS */

		/*
		 * Set hardware parameters.
		 */
		case __I2C_PLAT_INIT_HW:
			return __i2cSetParameters(dv);

		/*
		 * Set speed.
		 */
		case __I2C_PLAT_SET_SPEED:
			return __i2cSetSpeed(dv, param);

		/*
		 * Start reception.
		 */
		case __I2C_PLAT_INIT_RX:
			/* Use params->rxstep to cycle through a state machine in case
			 * we have to send sub-address.  */
			params->rxstep = I2C_RXSTATE_RESET;

			/* Reset sub-address sent counter */
			params->subaddr_cnt = 0;

			I2C_AcknowledgeConfig(params->base_addr, ENABLE);

			/*
			 * FALL
			 */

		/*
		 * Start transmission.
		 */
		case __I2C_PLAT_INIT_TX:
			/* Transmitting the START condition will chain all the interrupts
			 * needed for transmission
			 */
			I2C_ITConfig(params->base_addr, I2C_IT_EVT | I2C_IT_BUF, ENABLE);
			I2C_GenerateSTART(params->base_addr, ENABLE);
			return __DEV_OK;

		/*
		 * End reception.
		 */
		case __I2C_PLAT_END_RX:
			/*
			 * FALL
			 */

		/*
		 * End transmission.
		 */
		case __I2C_PLAT_END_TX:
			/* Ending DATA TX */
			// I2C_ITConfig(params->base_addr, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
			return __DEV_OK;

		/*
		 * Configure interrupts.
		 */
		case __I2C_PLAT_SET_IRQ:
#if __PLATI2C_MODE_INTERRUPTS
		{
			NVIC_InitTypeDef NVIC_InitStructure;

			/* Initial IRQ mapping */
			__intSetVector(dv->dv_txint,__i2cISR,dv);

			/* 	SET the NVIC channel */
			NVIC_InitStructure.NVIC_IRQChannel = dv->dv_txint;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&NVIC_InitStructure);
			I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, ENABLE);
		}
#endif /* __PLATI2C_MODE_INTERRUPTS */
			return __DEV_OK;

		/*
		 * Disable interrupts.
		 */
		case __I2C_PLAT_RESET_IRQ:
#if __PLATI2C_MODE_INTERRUPTS
		{
			NVIC_InitTypeDef NVIC_InitStructure;

			/* Disable I2C interrupts */
			I2C_ITConfig(params->base_addr, I2C_IT_BUF | I2C_IT_EVT, DISABLE);

			NVIC_InitStructure.NVIC_IRQChannel = dv->dv_txint;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
			NVIC_Init(&NVIC_InitStructure);

			__intSetVector(dv->dv_txint,__NULL,__NULL);
		}
#endif /* __PLATI2C_MODE_INTERRUPTS */
			return __DEV_OK;

		/*
		 * Transmit remote address.
		 */
		case __I2C_PLAT_SET_ADDRESS:
			/* Transmit remote address */
			if ((u8) param & 0x01)
			{
				I2C_Send7bitAddress(params->base_addr, (u8) param, I2C_Direction_Receiver);
			} else {
				I2C_Send7bitAddress(params->base_addr, (u8) param, I2C_Direction_Transmitter);
			}
			return __DEV_OK;

		/*
		 * Configure our own address.
		 */
		case __I2C_PLAT_SET_OWN_ADDRESS:
			/* Configure our own address */
			I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
			I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
			I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
			I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
			I2C_InitStructure.I2C_ClockSpeed = params->default_speed;
			I2C_InitStructure.I2C_OwnAddress1 = (u16) param;
			I2C_Init(params->base_addr, &I2C_InitStructure);
			return __DEV_OK;

		/*
		 * Generate a start bit.
		 */
		case __I2C_PLAT_GENERATE_START:
			/* Generate START condition */
			I2C_GenerateSTART(params->base_addr, ENABLE);
			return __DEV_OK;

		/*
		 * Generate a stop bit.
		 */
		case __I2C_PLAT_GENERATE_STOP:
			/* Generate STOP condition */
			I2C_GenerateSTOP(params->base_addr, ENABLE);
			return __DEV_OK;

		/*
		 * Check for an event.
		 */
		case __I2C_PLAT_CHECK_EVENT:

			/* Wait for a given event (param parameter) */

			time = __systemGetTickCount();
			while (__systemGetTickCount() - time < 1000)
			{
				switch (param)
				{
					case __I2C_EVENT_MASTER_MODE:
						if (I2C_CheckEvent(params->base_addr, I2C_EVENT_MASTER_MODE_SELECT))
							return __DEV_OK;

						break;

					case __I2C_EVENT_MASTER_TX:
						if (I2C_CheckEvent(params->base_addr, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
							return __DEV_OK;

						break;

					case __I2C_EVENT_MASTER_RX:
						if (I2C_CheckEvent(params->base_addr, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
							return __DEV_OK;

						break;

					case __I2C_EVENT_BYTE_TRANSMITTED:
						if (I2C_CheckEvent(params->base_addr, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
							return __DEV_OK;

						break;

					case __I2C_EVENT_BYTE_RECEIVED:
						if (I2C_CheckEvent(params->base_addr, I2C_EVENT_MASTER_BYTE_RECEIVED))
							return __DEV_OK;

						break;
				}

				__threadYield();
			}
			return __DEV_TIMEOUT;

		/*
		 * Configure ACK generation.
		 */
		case __I2C_PLAT_SET_ACK:
			I2C_AcknowledgeConfig(params->base_addr, (FunctionalState) param);
			return __DEV_ERROR;

		/*
		 * Write single byte.
		 */
		case __I2C_PLAT_WRITE_BYTE:
			I2C_SendData(params->base_addr, (u8) param);
			return __DEV_OK;

		/*
		 * Read a single byte.
		 */
		case __I2C_PLAT_READ_BYTE:
			return I2C_ReceiveData(params->base_addr);

		/*
		 * Enable device.
		 */
		case __I2C_PLAT_ENABLE_DEVICE:
			return __DEV_OK;

		/*
		 * Disable device.
		 */
		case __I2C_PLAT_DISABLE_DEVICE:
			return __DEV_OK;

		/*
		 * Abort condition.
		 */
		case __I2C_PLAT_ABORT:
			/* Abort transaction, reset device */
			__i2cSetParameters(dv);
			return __DEV_OK;
	}

	return __DEV_UNK_IOCTL;
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_I2C */

