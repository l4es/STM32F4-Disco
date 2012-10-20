/***************************************************************************
 * terminal.h
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

#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <plat_cpu.h>
#include "device.h"
#include "lock.h"

#if __CONFIG_COMPILE_TERMINAL

/*
 * Advance terminal structure declaration.
 */
struct __terminalTag;

/** @addtogroup Terminal
  * @{
  */

/** @defgroup Terminal_FunctionPrototypes Prototypes
  * @{
  */

/*!
 * @brief Callback function to receive input terminal commands.
 *
 * @param cmd		Pointer to a null-terminated string indicating the received command.
 * @param params	Pointer to a null-terminated string indicating the received parameters for
 * 					the \c cmd command.
 *
 * @return Nothing.
 *
 */
typedef __VOID (__TERMINALRXCMD)(struct __terminalTag* term, __PSTRING cmd, __PSTRING params);

/**
  * @}
  */

/** @defgroup Terminal_Defines Constants
  * @{
  */

/** @defgroup Terminal_Defaults Default values
  * @{
  */

#define __TERMINAL_DEF_LINELEN		64

/**
  * @}
  */

/** @defgroup Terminal_Flags Flags
  * @{
  */

#define	__TERMINAL_ECHO_ENABLED		0x01
#define __TERMINAL_DEBUG_COMMANDS	0x02
#define __TERMINAL_STARTED			0x04

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup Terminal_Typedefs Typedefs
  * @{
  */

typedef struct __terminalCommandTag
{
	__PSTRING cmd;
	__TERMINALRXCMD* func;
	struct __terminalCommandTag* next;

} __TERMINALCMD, *__PTERMINALCMD;

typedef struct __terminalTag
{
	__PSTRING name;							/*!< @brief Terminal name. */
	__PDEVICE dv_in;						/*!< @brief Input device. */
	__PDEVICE dv_out;						/*!< @brief Output device. */
	__PSTRING tx_line;						/*!< @brief TX line buffer. */
	__PSTRING rx_line;						/*!< @brief RX line buffer. */
	__PSTRING prompt;						/*!< @brief Custom terminal prompt. */
	u16	linelen;							/*!< @brief Line length. */
	u16 rxoffs;								/*!< @brief RX line offset. */
	u16	cursor;								/*!< @brief Cursor position. */
	__PLOCK lock;							/*!< @brief Write lock. */
	u8 flags;								/*!< @brief Flags. */
	__PTERMINALCMD cmds;					/*!< @brief Registered commands for this terminal. */
} __TERMINAL, *__PTERMINAL;

/**
  * @}
  */

__PTERMINAL __terminalCreate(__PSTRING name, __PDEVICE in, __PDEVICE out, u16 linelen, __PSTRING prompt, u8 flags);
__BOOL __terminalStart(__PTERMINAL term);
__VOID __terminalWrite(__PTERMINAL term, __CONST __PSTRING str, ...);
__VOID __terminalWriteLine(__PTERMINAL term, __CONST __PSTRING str, ...);
__VOID __terminalOut(__PTERMINAL term, __BOOL crlf, __CONST __PSTRING str, __PSTRING args);
__VOID __terminalAddCommand(__PTERMINAL term, __PTERMINALCMD cmd, u8 count);
__VOID __terminalCreateCommand(__PTERMINAL term, __PSTRING cmd, __TERMINALRXCMD* func);

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_TERMINAL */

#endif /* __TERMINAL_H__ */
