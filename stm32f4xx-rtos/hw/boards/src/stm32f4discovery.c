/***************************************************************************
 * stm32f4discovery.c
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

#include <plat_config.h>

#ifdef BOARD_STM32F4DISCOVERY

#include "stm32f4discovery.h"
#include <drivers/inc/serial.h>
#include <drivers/inc/spi.h>
#include <drivers/inc/i2c.h>

#if __CONFIG_COMPILE_SERIAL

/*
 * Example of lower-layer driver definition for
 * USART1, for the stm32f4xx, on the STM32F4DISCOVERY board
 * --------------------------------------------------------------------------
 *
 * The plat_uart.c helper will use the serialDevices array when indexing the
 * available uarts (that may vary from board to board).
 *
 * We are defining the available uarts this way. However they can be
 * initialized at runtime using __boardInitHW(). This is done because the
 * stm32 can remap it's ports to different pins, and that could be a difference
 * between boards. The same applies for the rest of the implemented device
 * drivers.
 *
 * If you are implementing another board for a stm32 mcu you can copy this
 * initialization and change it.
 *
 * This is naturally a characteristic of the default base implementation
 * for Milos device drivers. It is not said that ALL the drivers helpers
 * bound to a platform should be implemented this way. You can even define
 * constants for parameters for example, in the  stm32f4discovery.h, and reference
 * these constants directly in your plat_xxxxx.c driver (without defining
 * and array like the drivers implemented below).
 *
 * It is important to make the __PDEVICE pointers visible to the upper layers.
 * For example to "map" serialDevices[0] to __SERIAL_1
 *
 * The file stm32f4discovery.h declares:
 * #define __SERIAL_1 &serialDevices[0]
 *
 * The file plat_uart.h defines:
 * extern __DEVICE serialDevices[BOARD_UART_COUNT];
 *
 * What to do when adding/modifying/removing an uart:
 * 	-	Change BOARD_UART_COUNT if necessary.
 * 	-	Change the parameters constants (like BOARD_UART1_TX_PIN, etc.)
 * 	-	Change the uartParams array.
 * 	- 	Change the serialDevices array.
 */

/*
 * Define UART location for the STM32F4DISCOVERY board
 */

/* UART count for STM32F4DISCOVERY */
#define BOARD_UART_COUNT				1

/* UART1 parameters */
#define	BOARD_UART1_IRQ					37
#define BOARD_UART1_TX_PIN				GPIO_Pin_9
#define BOARD_UART1_RX_PIN				GPIO_Pin_10
#define BOARD_UART1_PORT				GPIOA
#define BOARD_UART1_BASE_REG			USART1
#define BOARD_UART1_APB_BUS				2
#define BOARD_UART1_BUS_ADDR			RCC_APB2Periph_USART1
#define BOARD_UART1_PORT_BUS_ADDR		RCC_AHB1Periph_GPIOA
#define BOARD_UART1_REMAP_VALUE			0

__STATIC __SERIAL_PDB serialPdb[BOARD_UART_COUNT];
__STATIC __EVENT serialTxEvts[BOARD_UART_COUNT];
__STATIC __EVENT serialRxEvts[BOARD_UART_COUNT];

/* Definition of the UART1.
 * Each UART in the STM32F4DISCOVERY board should have a definition
 * like the following.
 */
__STATIC UART_PARAMS uartParams[BOARD_UART_COUNT] = {
	{
		BOARD_UART1_TX_PIN,
		BOARD_UART1_RX_PIN,
		BOARD_UART1_PORT,
		BOARD_UART1_BASE_REG,
		BOARD_UART1_BUS_ADDR,
		BOARD_UART1_APB_BUS,
		BOARD_UART1_REMAP_VALUE
	}
};

__STATIC __DEVICE serialDevices[BOARD_UART_COUNT] = {
		{
			"serial1",
			__DEV_USART,
			0,
			BOARD_UART1_IRQ,
			BOARD_UART1_IRQ,
			0,
			0,
			&serialTxEvts[0],		// TX event manager
			&serialRxEvts[0],		// RX event manager
			__NULL,
			&serialPdb[0],
			&uartParams[0],
			__NULL,					// Pointer to next device driver
			__serialInit,			// Device init function
			__serialDeinit,			// Device destroy function
			__serialIOCtl,			// Device IO control function
			__serialOpen,			// Device open function
			__serialClose,			// Device close function
			__serialRead,			// Device read function
			__serialWrite,			// Device write function
			__serialFlush,			// Device flush function
			__serialSize,			// Device size function
			__serialPlatIoCtl,		// Platform-related IO control */
		}
};

#endif // __CONFIG_COMPILE_SERIAL

#if __CONFIG_COMPILE_SPI

/*
 * Define SPI location for the STM32F4DISCOVERY board
 */
/* SPI count for STM32F4DISCOVERY */
#define BOARD_SPI_COUNT					1

/* SPI1 parameters */
#define BOARD_SPI1_IRQ					35

#define BOARD_SPI1_BASE_ADDR			SPI1
#define BOARD_SPI1_PORT_MISO			GPIOA
#define BOARD_SPI1_PIN_MISO				GPIO_Pin_6
#define BOARD_SPI1_PORT_MOSI			GPIOA
#define BOARD_SPI1_PIN_MOSI				GPIO_Pin_7
#define BOARD_SPI1_PORT_CLK				GPIOA
#define BOARD_SPI1_PIN_CLK				GPIO_Pin_5
#define BOARD_SPI1_PORT_CS				GPIOA
#define BOARD_SPI1_PIN_CS				GPIO_Pin_4
#define BOARD_SPI1_REMAP_VALUE			0
#define BOARD_SPI1_DIRECTION			SPI_Direction_2Lines_FullDuplex
#define BOARD_SPI1_PRESCALER			SPI_BaudRatePrescaler_256
#define BOARD_SPI1_CPOL					SPI_CPOL_Low
#define BOARD_SPI1_CPHA					SPI_CPHA_1Edge
#define BOARD_SPI1_NSS_MODE				SPI_NSS_Soft


/*
 * As with the UART definitions,we do the same
 * for SPI driver interface (definitions used by
 * plat_spi.c helper).
 */

__STATIC __SPI_PDB spiPdb[BOARD_SPI_COUNT];
__STATIC __EVENT spiTxEvts[BOARD_SPI_COUNT];
__STATIC __EVENT spiRxEvts[BOARD_SPI_COUNT];

__STATIC SPI_PARAMS spiParams[BOARD_SPI_COUNT] = {
	{
		BOARD_SPI1_BASE_ADDR,
		BOARD_SPI1_PORT_MISO,
		BOARD_SPI1_PIN_MISO,
		BOARD_SPI1_PORT_MOSI,
		BOARD_SPI1_PIN_MOSI,
		BOARD_SPI1_PORT_CLK,
		BOARD_SPI1_PIN_CLK,
		BOARD_SPI1_PORT_CS,
		BOARD_SPI1_PIN_CS,
		BOARD_SPI1_REMAP_VALUE,
		BOARD_SPI1_DIRECTION,
		BOARD_SPI1_PRESCALER,
		BOARD_SPI1_CPOL,
		BOARD_SPI1_CPHA,
		BOARD_SPI1_NSS_MODE,
	}
};

__STATIC __DEVICE spiDevices[BOARD_SPI_COUNT] = {
		{
			"spi1",
			__DEV_SPI,
			0,
			BOARD_SPI1_IRQ,
			BOARD_SPI1_IRQ,
			0,
			0,
			&spiTxEvts[0],		// TX event manager
			&spiRxEvts[0],		// RX event manager
			__NULL,
			&spiPdb[0],
			&spiParams[0],
			__NULL,				// Pointer to next device driver
			__spiInit,			// Device init function
			__spiDeinit,		// Device destroy function
			__spiIOCtl,			// Device IO control function
			__spiOpen,			// Device open function
			__spiClose,			// Device close function
			__spiRead,			// Device read function
			__spiWrite,			// Device write function
			__spiFlush,			// Device flush function
			__spiSize,			// Device size function
			__spiPlatIoCtl,
		}
};

#endif // __CONFIG_COMPILE_SPI

/*
 * I2C Driver for the STM32F4DISCOVERY board
 */
#if __CONFIG_COMPILE_I2C

/* I2C parameters */
#define BOARD_I2C_IRQ					31
#define BOARD_I2C_COUNT					1
#define BOARD_I2C_GPIO_PORT				GPIOB
#define BOARD_I2C_BUS_ADDR				RCC_APB1Periph_I2C1
#define BOARD_I2C_APB_NUM				1
#define BOARD_I2C_BASE_ADDR				I2C1
#define BOARD_I2C_PIN_SCL				GPIO_Pin_8
#define BOARD_I2C_PIN_SDA				GPIO_Pin_9
#define BOARD_I2C_DEFAULT_SPEED			10000

__STATIC __I2C_PDB i2cPdb[BOARD_I2C_COUNT];
__STATIC __EVENT i2cTxEvts[BOARD_I2C_COUNT];
__STATIC __EVENT i2cRxEvts[BOARD_I2C_COUNT];

__STATIC I2C_PARAMS i2cParams[BOARD_I2C_COUNT] = {
	{
		BOARD_I2C_PIN_SCL,
		BOARD_I2C_PIN_SDA,
		BOARD_I2C_GPIO_PORT,
		BOARD_I2C_BASE_ADDR,
		BOARD_I2C_BUS_ADDR,
		BOARD_I2C_APB_NUM,
		BOARD_I2C_DEFAULT_SPEED
	}
};

__STATIC __DEVICE i2cDevices[BOARD_I2C_COUNT] = {
		{
			"i2c1",
			__DEV_I2C,
			0,
			BOARD_I2C_IRQ,
			BOARD_I2C_IRQ,
			0,
			0,
			&i2cTxEvts[0],		// TX event manager
			&i2cRxEvts[0],		// RX event manager
			__NULL,
			&i2cPdb[0],
			&i2cParams[0],
			__NULL,				// Pointer to next device driver
			__i2cInit,			// Device init function
			__i2cDeinit,		// Device destroy function
			__i2cIOCtl,			// Device IO control function
			__i2cOpen,			// Device open function
			__i2cClose,			// Device close function
			__i2cRead,			// Device read function
			__i2cWrite,			// Device write function
			__i2cFlush,			// Device flush function
			__i2cSize,			// Device size function
			__i2cPlatIoCtl,
		}
};

#endif // __CONFIG_COMPILE_I2C


/*!< @brief Custom board hardware initialization.
 *
 * If you are implementing a custom board, this is the only
 * function you should define.
 * Called from __cpuInitHardware() to initialize
 * custom hardware for a specific board.
 *
 * @return 		Nothing.
 */
__VOID __boardInitHW(__VOID) {

	GPIO_InitTypeDef 	GPIO_InitStructure;

	/*****************************************/
	/* Configure and enable clock for LEDs   */
	/*****************************************/

	/* Configure input/output/alternative function */
	GPIO_InitStructure.GPIO_Pin = (	GPIO_Pin_12 | GPIO_Pin_13 |
									GPIO_Pin_14 | GPIO_Pin_15);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* Enable clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	/*****************************************/
	/* Configure and enable clock for KEY    */
	/*****************************************/

	/* Configure input/output/alternative function */
	GPIO_InitStructure.GPIO_Pin = (	GPIO_Pin_0);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Enable clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* Add every board __DEVICE */
#if __CONFIG_COMPILE_SERIAL
	__deviceAdd(serialDevices, BOARD_UART_COUNT);
#endif /* __CONFIG_COMPILE_SERIAL */

#if __CONFIG_COMPILE_SPI
	__deviceAdd(spiDevices, BOARD_SPI_COUNT);
#endif /* __CONFIG_COMPILE_SPI */

#if __CONFIG_COMPILE_I2C
	__deviceAdd(i2cDevices, BOARD_I2C_COUNT);
#endif /* __CONFIG_COMPILE_I2C */

}

#endif /* BOARD_STM32F4_DISCOVERY */
