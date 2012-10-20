/***************************************************************************
 * plat_uart.c
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

#include "plat_uart.h"
#include <drivers/inc/serial.h>
#include <core/inc/intrvect.h>

#if __CONFIG_COMPILE_SERIAL

__STATIC u16 __uartGetWordLenght(__PSERIAL_PDB pd)
{
	if (pd->pd_mode & __SERIAL_LENGTH_8) return USART_WordLength_8b;
	else if (pd->pd_mode & __SERIAL_LENGTH_9) return USART_WordLength_9b;
	else return 0;
}

__STATIC u16 __uartGetStopBits(__PSERIAL_PDB pd)
{
	if (pd->pd_mode & __SERIAL_STOPBITS_1) return USART_StopBits_1;
	else if (pd->pd_mode & __SERIAL_STOPBITS_0_5) return USART_StopBits_0_5;
	else if (pd->pd_mode & __SERIAL_STOPBITS_2) return USART_StopBits_2;
	else if (pd->pd_mode & __SERIAL_STOPBITS_1_5) return USART_StopBits_1_5;
	else return 0;
}

__STATIC u16 __uartGetStopParity(__PSERIAL_PDB pd)
{
	if (pd->pd_mode & __SERIAL_PARITY_NO) return USART_Parity_No;
	else if (pd->pd_mode & __SERIAL_PARITY_EVEN) return USART_Parity_Even;
	else if (pd->pd_mode & __SERIAL_PARITY_ODD) return USART_Parity_Odd;
	else return 0;
}

__STATIC u16 __uartGetFlowControl(__PSERIAL_PDB pd)
{
	if (pd->pd_mode & __SERIAL_FLOW_NONE) return USART_HardwareFlowControl_None;
	else if (pd->pd_mode & __SERIAL_FLOW_RTS) return USART_HardwareFlowControl_RTS;
	else if (pd->pd_mode & __SERIAL_FLOW_CTS) return USART_HardwareFlowControl_CTS;
	else if (pd->pd_mode & __SERIAL_FLOW_RTS_CTS) return USART_HardwareFlowControl_RTS_CTS;
	else return 0;
}

__STATIC i32 __uartSetPins(__PDEVICE dv)
{
	PUART_PARAMS params = dv->dv_params;
	u8 pin_source;

	/* 	Configure USART TX as alternate function push-pull */
	__cpuPinConfigure(params->tx_pin, params->port_addr, GPIO_Speed_50MHz, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL);

  	/* 	Configure USART RX as input floating */
	__cpuPinConfigure(params->rx_pin, params->port_addr, (GPIOSpeed_TypeDef) 0, GPIO_Mode_IN, GPIO_OType_PP, GPIO_PuPd_NOPULL);

	/* Select alternative function for IO pins */
	pin_source	= 0x00;
	while (params->tx_pin > (1<<pin_source)){
		pin_source++;
	}
	if ((params->base_addr == USART1)||(params->base_addr == USART2)||(params->base_addr == USART3)){
		GPIO_PinAFConfig(params->port_addr, pin_source, 0x07);	// GPIO_AF_USART1 = GPIO_AF_USART2 = GPIO_AF_USART3 = 0x07
	}else{
		GPIO_PinAFConfig(params->port_addr, pin_source, 0x08);	// GPIO_AF_UART4 = GPIO_AF_UART5 = GPIO_AF_USART6 = 0x08
	}

	pin_source	= 0x00;
	while (params->rx_pin > (1<<pin_source)){
		pin_source++;
	}
	if ((params->base_addr == USART1)||(params->base_addr == USART2)||(params->base_addr == USART3)){
		GPIO_PinAFConfig(params->port_addr, pin_source, 0x07);	// GPIO_AF_USART1 = GPIO_AF_USART2 = GPIO_AF_USART3 = 0x07
	}else{
		GPIO_PinAFConfig(params->port_addr, pin_source, 0x08);	// GPIO_AF_UART4 = GPIO_AF_UART5 = GPIO_AF_USART6 = 0x08
	}

  	/* APB clock */
	if (params->apb_bus_num == 2)
	{
		RCC_APB2PeriphClockCmd(params->bus_clock_addr, ENABLE);
	} else
	{
		RCC_APB1PeriphClockCmd(params->bus_clock_addr, ENABLE);
	}

	return __DEV_OK;
}

__STATIC i32 __uartSetClock(__PDEVICE dv)
{
	USART_ClockInitTypeDef USART_InitClock;
	PUART_PARAMS params = dv->dv_params;

	USART_InitClock.USART_Clock		= USART_Clock_Disable;
  	USART_InitClock.USART_CPOL 		= USART_CPOL_Low;
  	USART_InitClock.USART_CPHA 		= USART_CPHA_2Edge;
  	USART_InitClock.USART_LastBit 	= USART_LastBit_Disable;

	USART_ClockInit(params->base_addr, &USART_InitClock);

	return __DEV_OK;
}

__STATIC i32 __uartSetNVIC(__PDEVICE dv)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 	SET the SERIAL NVIC channel */
	NVIC_InitStructure.NVIC_IRQChannel 						= dv->dv_txint;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd 					= ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	return __DEV_OK;
}

__STATIC i32 __uartConfig(__PDEVICE dv)
{
	USART_InitTypeDef USART_InitStructure;
	__PSERIAL_PDB pd = dv->dv_pdb;
	PUART_PARAMS params = dv->dv_params;

	USART_InitStructure.USART_BaudRate 				= pd->pd_baud;
	USART_InitStructure.USART_WordLength 			= __uartGetWordLenght(pd);
	USART_InitStructure.USART_StopBits 				= __uartGetStopBits(pd);
	USART_InitStructure.USART_Parity 				= __uartGetStopParity(pd);
	USART_InitStructure.USART_HardwareFlowControl 	= __uartGetFlowControl(pd);
	USART_InitStructure.USART_Mode 					= USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(params->base_addr, &USART_InitStructure);

	return __DEV_OK;
}

__STATIC i32 __uartSetParameters(__PDEVICE dv)
{
	PUART_PARAMS params = dv->dv_params;

	__uartSetPins(dv);

	__uartSetNVIC(dv);

	__uartConfig(dv);

	__uartSetClock(dv);

	/* 	Enable the USART Receive interrupt: this interrupt is generated when the
   		USART receive data register is not empty */
  	USART_ITConfig(params->base_addr, USART_IT_RXNE, ENABLE);

  	/* 	Enable USART1 */
  	USART_Cmd(params->base_addr, ENABLE);
	return __DEV_OK;
}

__STATIC u8 __uartCharOutput(__PDEVICE dv, u8 c)
{
	USART_TypeDef* pSERIAL;
	pSERIAL = ((UART_PARAMS*) dv->dv_params)->base_addr;
	pSERIAL->DR = (c & (u16)0x01FF);
	return __DEV_OK;
}

__STATIC u8 __uartCharInput(__PDEVICE dv)
{
	USART_TypeDef* pSERIAL;
	pSERIAL = ((UART_PARAMS*) dv->dv_params)->base_addr;
	return pSERIAL->DR;
}

__VOID __uartISR(__PVOID pVoid)
{
	__PDEVICE dv;
	__PSERIAL_PDB pd;
	USART_TypeDef* pSERIAL;
	u32	irr;

	__systemEnterISR();

	dv = (__PDEVICE) pVoid;
	pd = dv->dv_pdb;

	pSERIAL = ((UART_PARAMS*) dv->dv_params)->base_addr;
	irr = pSERIAL->SR;

	if (irr & USART_FLAG_TC)
	{
		pSERIAL->SR &= ~USART_FLAG_TC;
	}

	if (irr & USART_FLAG_TXE)
	{
		pSERIAL->SR &= ~USART_FLAG_TXE;

		if (pd->pd_txcnt)
		{
			__uartCharOutput(dv, *(pd->pd_txbuf + pd->pd_tcidx));
			if (++pd->pd_tcidx >= pd->pd_txlen) pd->pd_tcidx = 0;
			--pd->pd_txcnt;
		} else
		{
			/* Nothing to send, disable TXE and set event */
			if (pSERIAL->CR1 & USART_FLAG_TXE)
			{
				pSERIAL->CR1 &= ~USART_FLAG_TXE;
				__eventSet(dv->dv_txev);
			}
		}
	}

	if (irr & USART_FLAG_RXNE)
	{
		*(pd->pd_rxbuf + pd->pd_rcidx) = __uartCharInput(dv);
		if (++pd->pd_rcidx >= pd->pd_rxlen) pd->pd_rcidx = 0;
		if (++pd->pd_rxcnt > pd->pd_rxlen) pd->pd_rxerr |= __SERIALERR_OVERFLOW;
		__eventSet(dv->dv_rxev);
	}

	__systemLeaveISR();
}

u8 __uartInitTx(__PDEVICE dv)
{
	USART_TypeDef* pSERIAL;
	pSERIAL = ((UART_PARAMS*) dv->dv_params)->base_addr;
	pSERIAL->CR1 |= USART_FLAG_TXE;

	return __DEV_OK;
}

/*
 * @brief UART for stm32 IO control function
 *
 * Called from @ref Serial driver to perform platform-related
 * tasks, like pin configuration or baudrate settings.
 *
 * @param	dv		Pointer to device.
 * @param	code	IO control code.
 *
 * @arg	__SERIAL_PLAT_INIT_HW			Initialize hardware.
 * @arg __SERIAL_PLAT_SET_BAUDRATE		Set baudrate.
 * @arg	__SERIAL_PLAT_CHAR_OUTPUT		Character output.
 * @arg	__SERIAL_PLAT_CHAR_INPUT		Character input.
 * @arg __SERIAL_PLAT_INIT_TX			Start transmission.
 * @arg __SERIAL_PLAT_SET_IRQ			Configure interrupts.
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
i32 __serialPlatIoCtl(__PDEVICE dv, u32 code, u32 param, __PVOID in, u32 in_len, __PVOID out, u32 out_len)
{

	switch (code)
	{
		case __SERIAL_PLAT_INIT_HW:
			return __uartSetParameters(dv);

		case __SERIAL_PLAT_CHAR_OUTPUT:
			return __uartCharOutput(dv, (u8) param);

		case __SERIAL_PLAT_CHAR_INPUT:
			*((u8*) out) = __uartCharInput(dv);
			out_len = 1;
			return __DEV_OK;

		case __SERIAL_PLAT_INIT_TX:
			return __uartInitTx(dv);

		case __SERIAL_PLAT_SET_IRQ:
			__intSetVector(dv->dv_txint,__uartISR,dv);
			return __DEV_OK;

		case __SERIAL_PLAT_RESET_IRQ:
			__intSetVector(dv->dv_txint,__NULL,__NULL);
			return __DEV_OK;

		case __SERIAL_PLAT_SET_PARITY:
		case __SERIAL_PLAT_SET_STOP_BITS:
		case __SERIAL_PLAT_SET_FLOW_CONTROL:
		case __SERIAL_PLAT_SET_LENGTH:
		case __SERIAL_PLAT_SET_BAUDRATE:
			return __uartConfig(dv);
	}

	return __DEV_UNK_IOCTL;
}

#endif // __CONFIG_COMPILE_SERIAL

