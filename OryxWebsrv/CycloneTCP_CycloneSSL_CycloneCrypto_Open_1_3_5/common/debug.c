/**
 * @file debug.c
 * @brief Debugging facilities
 *
 * @section License
 *
 * Copyright (C) 2010-2013 Oryx Embedded. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded (www.oryx-embedded.com)
 * @version 1.3.5
 **/

//Dependencies
#include <stdarg.h>
#include <stdio.h>

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "debug.h"

#define USART_BUFF_PRINTF_SIZE		2048
#define USART_QUEUE_TX_SIZE			2048

struct usart_queue
{
	volatile uint16_t p_rd;
	volatile uint16_t p_wr;
	uint8_t q[USART_QUEUE_TX_SIZE];
};

volatile uint8_t _tx_primed;
char _usart_printf_buff[USART_BUFF_PRINTF_SIZE];
struct usart_queue _uart6_tx_queue;


/**
 * @brief Debug UART initialization
 * @param[in] baudrate UART baudrate
 **/
void debugInit()
{
	_uart6_tx_queue.p_rd = 0;
	_uart6_tx_queue.p_wr = 0;
	_tx_primed = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	//Enable GPIO clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	//Enable USART6 clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	//Remap TX and RX pins to PC6 and PC7
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
	//GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);	//fixme

	GPIO_InitTypeDef gpi;
	gpi.GPIO_Pin = GPIO_Pin_6  /* | GPIO_Pin_7 */;	//fixme
	gpi.GPIO_Mode = GPIO_Mode_AF;
	gpi.GPIO_Speed = GPIO_Speed_50MHz;
	gpi.GPIO_OType = GPIO_OType_PP;
	gpi.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &gpi);


	USART_InitTypeDef uis;
	uis.USART_BaudRate = 115200;
	uis.USART_WordLength = USART_WordLength_8b;
	uis.USART_StopBits = USART_StopBits_1;
	uis.USART_Parity = USART_Parity_No;
	uis.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uis.USART_Mode = USART_Mode_Tx /* | USART_Mode_Rx */;	//fixme
	USART_Init(USART6, &uis);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef nis;
	nis.NVIC_IRQChannel = USART6_IRQn;
	nis.NVIC_IRQChannelPreemptionPriority = 2;
	nis.NVIC_IRQChannelSubPriority = 0;
	nis.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nis);

	USART_Cmd(USART6, ENABLE);
}

static inline uint8_t usart_queue_full(struct usart_queue* q)
{
	return(((q->p_wr + 1) % USART_QUEUE_TX_SIZE) == q->p_rd);
}

static inline uint8_t usart_queue_empty(struct usart_queue* q)
{
	return(q->p_wr == q->p_rd);
}

static uint8_t usart_enqueue(struct usart_queue* q, uint8_t data)
{
	if(usart_queue_full(q))
		return 0;
	else
	{
		q->q[q->p_wr] = data;
		q->p_wr = ((q->p_wr + 1) == USART_QUEUE_TX_SIZE) ?
			0 : q->p_wr + 1;
	}

	return 1;
}

static uint8_t usart_dequeue(struct usart_queue* q, uint8_t* data)
{
	if(usart_queue_empty(q))
		return 0;
	else
	{
		*data = q->q[q->p_rd];
		q->p_rd = ((q->p_rd + 1) == USART_QUEUE_TX_SIZE) ?
			0 : q->p_rd + 1;
	}

	return 1;
}

void usart_putc(const char c)
{
	while(!usart_enqueue(&_uart6_tx_queue, c));

	if(! _tx_primed)
	{
		_tx_primed = 1;
		USART_ITConfig(USART6, USART_IT_TXE, ENABLE);
	}
}

void usart_puts(const char* s)
{
	while(*s)
		usart_putc(*s++);
}

void usart_printf(const char *pFormat, ...)
{
    va_list ap;

    va_start(ap, pFormat);
    vsprintf(_usart_printf_buff, pFormat, ap);
    va_end(ap);
    usart_puts(_usart_printf_buff);
}

/**
 * @brief Display the contents of an array
 * @param[in] stream Pointer to a FILE object that identifies an output stream
 * @param[in] prepend String to prepend to the left of each line
 * @param[in] data Pointer to the data array
 * @param[in] length Number of bytes to display
 **/

void debugDisplayArray(FILE *stream,
   const char_t *prepend, const void *data, uint_t length)
{
   uint_t i;

   for(i = 0; i < length; i++)
   {
      //Beginning of a new line?
      if((i % 16) == 0)
    	  usart_puts(prepend);
      //Display current data byte
      usart_printf("%02X ", *((unsigned char *) data + i));
      //End of current line?
      if((i % 16) == 15 || i == (length - 1))
    	  usart_printf("\r\n");
   }
}

void USART6_IRQHandler(void)
{
	if(USART_GetITStatus(USART6, USART_IT_TXE) != RESET)
	{
		uint8_t data;

		/* Write one byte to the transmit data register */
		if(usart_dequeue(&_uart6_tx_queue , &data))
		{
			USART_SendData(USART6, data);
		}
		else
		{
			// if we have nothing to send , disable the interrupt
			// and wait for a kick
			USART_ITConfig(USART6, USART_IT_TXE , DISABLE);
			_tx_primed = 0;
		}
	}
}
