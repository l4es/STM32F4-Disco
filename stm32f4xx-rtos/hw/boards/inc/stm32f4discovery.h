/***************************************************************************
 * stm32f4discovery.h
 * (C) 2012 Truong-Khang LE
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

#ifndef __STM32F4DISCOVERY_H__
#define __STM32F4DISCOVERY_H__

/* SPI default buffer sizes */
#define BOARD_SPI_RXBUFLEN				600
#define BOARD_SPI_TXBUFLEN				600

/* Heartbeat led */
#define BOARD_HEARTBEAT_LED				GPIO_Pin_15
#define BOARD_HEARTBEAT_GPIO			GPIOD

/* Leds */
#define BOARD_LED_COUNT					3
#define BOARD_GET_LED_PORT(x,p)			(p = GPIOD)
#define BOARD_GET_LED_PIN(x,p)			(p = (GPIO_Pin_14 << x))

/* Keys */
#define BOARD_KEY_COUNT					1
#define BOARD_GET_KEY_PORT(x,p)			\
	switch (x)							\
	{									\
		case 1: p = GPIOA; break;		\
		default: p = __NULL; break;		\
	}

#define BOARD_GET_KEY_PIN(x,p)			\
	switch (x)							\
	{									\
		case 1: p = GPIO_Pin_0; break;	\
		default: p = GPIO_Pin_0; break;	\
	}

#endif // __STM32F4DISCOVERY_H__
