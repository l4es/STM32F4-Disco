/***************************************************************************
 * terminal.c
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

#include "terminal.h"
#include "lock.h"
#include "heap.h"
#include <common/inc/string.h>
#include <common/inc/mem.h>
#include "device.h"
#include "system.h"
#include "dbgterm.h"

#if (__CONFIG_COMPILE_TERMINAL && __CONFIG_COMPILE_STRING)

/** @addtogroup Core
  * @{
  */

/** @defgroup Terminal Terminal
  * Terminal functions.
  * @{
  */

/** @defgroup Terminal_PrivateConstants Private constants
  * @{
  */

#define __SK_BASE			0x1000
#define __SK_UNKNOWN		0xFFFF
#define __SK_ESCAPE			__SK_BASE
#define __SK_UPARROW		__SK_BASE + 1
#define __SK_DOWNARROW		__SK_BASE + 2
#define __SK_RIGHTARROW		__SK_BASE + 3
#define __SK_LEFTARROW		__SK_BASE + 4
#define __SK_BREAK			__SK_BASE + 5
#define __SK_INS			__SK_BASE + 6
#define __SK_HOME			__SK_BASE + 7
#define __SK_PGUP			__SK_BASE + 8
#define __SK_DEL			__SK_BASE + 9
#define __SK_END			__SK_BASE + 10
#define __SK_PGDN			__SK_BASE + 11
#define __SK_F1				__SK_BASE + 12
#define __SK_F2				__SK_BASE + 13
#define __SK_F3				__SK_BASE + 14
#define __SK_F4				__SK_BASE + 15
#define __SK_F5				__SK_BASE + 16
#define __SK_F6				__SK_BASE + 17
#define __SK_F7				__SK_BASE + 18
#define __SK_F8				__SK_BASE + 19
#define __SK_F9				__SK_BASE + 20
#define __SK_F10			__SK_BASE + 21
#define __SK_F11			__SK_BASE + 22
#define __SK_F12			__SK_BASE + 23
#define	__TERMINAL_PROMPT	"milos> "

/**
  * @}
  */


/** @defgroup Terminal_PrivateVariables Private variables
  * @{
  */

__CONST __STRING __terminalESC[] 	= {0x1b, 0x5b};
__CONST __STRING __terminalBackspace[] 	= {0x1b, 0x5b, '1', 'D', 0x1b, 0x5b, '0', 'K', 0x00};
__CONST __STRING __terminalClearScreen[] = {0x1b, 0x5b, '2', 'J', 0x00};
__CONST __STRING __terminalShowCursor[]  = {0x1b, 0x5b, '?', '2', '5', 'h', 0x00};
__CONST __STRING __terminalHideCursor[]  = {0x1b, 0x5b, '?', '2', '5', 'l', 0x00};

#if __CONFIG_COMPILE_DBGTERM

__TERMINALCMD __dbgCommands[] = {
		{ "heap", 		__dbgTerminalIn, __NULL},
		{ "locks", 		__dbgTerminalIn, __NULL},
		{ "threads",	__dbgTerminalIn, __NULL},
		{ "reset", 		__dbgTerminalIn, __NULL},
		{ "uptime",		__dbgTerminalIn, __NULL},
		{ "devices",	__dbgTerminalIn, __NULL},
		{ "defrag",		__dbgTerminalIn, __NULL},

#if __CONFIG_COMPILE_FAT
		{ "dir",		__dbgTerminalIn, __NULL},
#endif /* __CONFIG_COMPILE_FAT */

#if __CONFIG_COMPILE_NET
		{ "ping",		__dbgTerminalIn, __NULL},
		{ "arp",		__dbgTerminalIn, __NULL},
		{ "netif",		__dbgTerminalIn, __NULL}
#endif /* __CONFIG_COMPILE_NET */
};

#endif /* __CONFIG_COMPILE_DBGTERM */

/**
  * @}
  */


/** @defgroup Terminal_Functions Functions
  * @{
  */

/*!
 * @brief Sends the cursor backwards
 *
 *	@param	term		Pointer to terminal structure.
 *	@param	qty			Quantity of spaces.
 *	@return				Nothing.
 *
 */
__STATIC __VOID __terminalBack(__PTERMINAL term, u8 qty)
{
	__STRING num[4];
	__STRING buf[sizeof(num) + 5];
	u8 len;

	__strFmt(num, "%u", qty);
	len = __strLen(num);
	
	buf[0] = __terminalESC[0];
	buf[1] = __terminalESC[1];
	__memCpy(buf+2, num, len);
	buf[2+len] = 'D';
	buf[3+len] = 0x00;
	
	__terminalWrite(term, buf);
} 

/*!
 * @brief Sends the cursor forward.
 *
 * @param	term		Pointer to terminal structure.
 * @param	qty			Quantity of spaces.
 * @return				Nothing.
 *
 */
__STATIC __VOID __terminalFoward(__PTERMINAL term, u8 qty)
{
	__STRING num[4];
	__STRING buf[sizeof(num) + 5];
	u8 len;

	__strFmt(num, "%u", qty);
	len = __strLen(num);

	buf[0] = __terminalESC[0];
	buf[1] = __terminalESC[1];
	__memCpy(buf+2, num, len);
	buf[2+len] = 'C';
	buf[3+len] = 0x00;
	
	__terminalWrite(term, buf);
}

/*!
 * @brief Shows prompt
 *
 * @param	term		Pointer to terminal structure.
 *
 * @return				Nothing.
 *
 */
__STATIC __VOID __terminalShowPrompt(__PTERMINAL term)
{
	if (__lockOwn(term->lock, 1000))
	{
		__deviceWrite(term->dv_out, term->prompt, __strLen(term->prompt));
		__deviceFlush(term->dv_out);
		__lockRelease(term->lock);
	}
}

/*!
 * @brief Parses escaped characters.
 *
 * Called after receiving an ESC character will read consecutively
 * until a known escape sequence is found.
 *
 * @param term	Pointer to a terminal structure.
 *
 * @return	Known key code value, otherwise __SK_UNKNOWN.
 *
 */
__STATIC u16 __terminalParseEscaped(__PTERMINAL term)
{
	u8	a = 0;
	u8	b = 0;
	u8 	c = 0;
	u8	d = 0;

	/* Check for 0x5b */
	if (__deviceRead(term->dv_in, &a, 1) == 1)
	{
		/* Not 0x5b? Return and save last char received */
		if (a != 0x5b)
		{
			return a;
		}
	} else
	{
		return __SK_ESCAPE;
	}
							
	/* 0x5b received, check for special chars */
	if (__deviceRead(term->dv_in, &b, 1) == 1)
	{
		switch (b) {
			/* Arrows */
			case 0x41:	
				return __SK_UPARROW;
			case 0x42:	
				return __SK_DOWNARROW;
			case 0x43:	
				return __SK_RIGHTARROW;
			case 0x44:	
				return __SK_LEFTARROW;
									
			/* Break */
			case 0x50:	
				return __SK_BREAK;		
				
												
			/* Four and five char codes */
			case 0x5b:
			case 0x31:
			case 0x32:
			case 0x33:
			case 0x35:
			case 0x36:
			case 0x37:	
				break;				
			default:
				return __SK_UNKNOWN;
		}
		
		if (__deviceRead(term->dv_in, &c, 1) == 1)
		{
			/* Four characters codes ending with 0x7e*/
			if (c == 0x7e) {
				switch (b) {
					case 0x31:
						return __SK_HOME;
					case 0x32:
						return __SK_INS;
					case 0x33:
						return __SK_DEL;
					case 0x34:
						return __SK_END;
					case 0x35:
						return __SK_PGUP;
					case 0x36:
						return __SK_PGDN;						
					default:
						return __SK_UNKNOWN;
				}
			}
			
			if (b != 0x31 && b != 0x32) return __SK_UNKNOWN;
			
			/* Five characters codes */
			if (__deviceRead(term->dv_in, &d, 1) == 1)
			{
				if (d != 0x7e) return __SK_UNKNOWN;
				
				switch (b) {
					case 0x31:
						switch (c) {
							case 0x31:
								return __SK_F1;
							case 0x32:
								return __SK_F2;
							case 0x33:
								return __SK_F3;
							case 0x34:
								return __SK_F4;
							case 0x35:
								return __SK_F5;
							case 0x37:
								return __SK_F6;
							case 0x38:
								return __SK_F7;
							case 0x39:
								return __SK_F8;
						}
						return __SK_UNKNOWN;
						
					case 0x32:
						switch (c) {
							case 0x30:
								return __SK_F9;
							case 0x31:
								return __SK_F10;
							case 0x33:
								return __SK_F11;
							case 0x34:
								return __SK_F12;
						}
						return __SK_UNKNOWN;
						
					default:
						return __SK_UNKNOWN;
				}
			}
		}
	}
	
	return __SK_UNKNOWN;
}

/*!
 * @brief Process key codes.
 *
 * @param term Pointer to a terminal structure.
 * @param code Key code (see @ref Terminal_PrivateConstants).
 *
 * @return	__TRUE if the key code has to be echoed, otherwise __FALSE.
 *
 */
__STATIC __BOOL __terminalParseSpecial(__PTERMINAL term, u16 code) {

	if (code == __SK_UNKNOWN) return __FALSE;
	
	switch (code)
	{
		case __SK_LEFTARROW:		if (term->cursor)
									{
										__terminalBack(term, 1);
										/* back cursor */
										term->cursor--;
									}
									break;
									
		case __SK_RIGHTARROW:		if (__strLen(term->rx_line) > term->cursor)
									{
										__terminalFoward(term, '1');
										
										/* back cursor */
										term->cursor++;
									}
									break;
									
		case __SK_UPARROW:
									break;
									
		case __SK_END:				/* hide cursor */
									__terminalWrite(term, __terminalHideCursor);
		
									/* move cursor */
									__terminalBack(term, term->cursor);
									
									term->cursor = 0;
									
									/* show cursor */
									__terminalWrite(term, __terminalShowCursor);
									break;
									
		case __SK_HOME:				if (term->cursor == __strLen(term->rx_line)) break;
									
									/* hide cursor */
									__terminalWrite(term, __terminalHideCursor);
		
									/* move cursor */
									__terminalFoward(term, __strLen(term->rx_line) - term->cursor);
									
									term->cursor = __strLen(term->rx_line);
									
									/* show cursor */
									__terminalWrite(term, __terminalShowCursor);
									break;								
	}
	
	return __FALSE;

}

/*!
 * @brief Outputs a character.
 *
 * @param	term	Pointer to a __TERMINAL structure.
 * @param	c 		Character to write.
 *
 * @return	Nothing.
 *
 */
__STATIC __VOID __terminalWriteChar(__PTERMINAL term, u8 c)
{
	if (!(term->flags & __TERMINAL_STARTED)) return;

	if (__lockOwn(term->lock, 1000)) {
		__deviceWrite(term->dv_out, &c, 1);
		__deviceFlush(term->dv_out);
		__lockRelease(term->lock);
	}
}

/*!
 * @brief Reads a single line of text.
 *
 * Called after receiving an ESC character will read consecutively
 * until a known escape sequence is found.
 *
 * @return	Known key code value, otherwise __SK_UNKNOWN.
 *
 */
__STATIC __BOOL __terminalReadLine(__PTERMINAL term)
{
	u16	size = 0;
	u8 	c = 0;
	u8 	ret = 0;
	u8 	echo;
	u8 	save;
	u8 	i = 0;
	u8	print = 0;

	for (;;)
	{
		size = __deviceSize(term->dv_in, __DEV_RXSIZE);
		while (size > 0)
		{
			if (__deviceRead(term->dv_in, &c, 1) == 1)
			{
				size--;

				echo 	= __FALSE;
				save 	= __TRUE;
				ret		= __FALSE;

				/* Parse received char */
				switch (c) {

					/* ESCAPE */
					case 0x1b:
						save = __FALSE;
						echo = __terminalParseSpecial(term, __terminalParseEscaped(term));
						
						/* Refresh size */
						size = __deviceSize(term->dv_in, __DEV_RXSIZE);
						break;

					/* TAB */
					case 0x09:
						save = __FALSE;
						echo = __FALSE;
						break;

					/* BACKSPACE */
					case 0x08:
						save = __FALSE;
						echo = __FALSE;

						if (!term->rxoffs) break;
						if (!term->cursor) break;

						/* Deleting a char between others */
						if (term->cursor < term->rxoffs)
						{
							for (i = term->cursor; i < term->rxoffs; i++)
							{
								term->rx_line[i-1] = term->rx_line[i];
							}
							
							print = __TRUE;
						}
						
						term->rxoffs--;
						term->cursor--;
						
						term->rx_line[term->rxoffs] = 0;
						
						if (print)
						{
							/* hide cursor */
							__terminalWrite(term, __terminalHideCursor);
						
							/* write a left and clear escape sequence */
							__terminalWrite(term, __terminalBackspace);
						
							/* rewrite the rest of the line */
							__terminalWrite(term, term->rx_line + term->cursor);
							
							/* move cursor back */
							__terminalBack(term, term->rxoffs - term->cursor);
							
							/* show cursor */
							__terminalWrite(term, __terminalShowCursor);
						} else
						{
							/* write a left and clear escape sequence */
							__terminalWrite(term, __terminalBackspace);
						}
						break;

					/* CR */
					case 0x0D:
						save = __FALSE;
						echo = __FALSE;

						/* Send back a LF */
						__terminalWrite(term, "\r\n");
						ret = __TRUE;
						break;

					/* LF (ignore) */
					case 0x0A:
						save = __FALSE;
						break;

					default:
						echo = __TRUE;
						break;
				}

				/* Buffer received char? */
				if (save) {
					/* Overflow? */
					if (term->rxoffs < term->linelen-1)
					{
						print = __FALSE;
					
						/* inserting char? */
						if (term->cursor < term->rxoffs)
						{
							for (i = term->linelen - 1; i > term->cursor; i--)
							{
								term->rx_line[i] = term->rx_line[i-1];
							}
							
							print = __TRUE;
						} 
						
						term->rx_line[term->cursor] = c;
					
						/* advance buffer ptr */
						term->rxoffs++;
						
						/* advance cursor */
						term->cursor++;

						/* ensure last byte is __NULL */
						term->rx_line[term->linelen - 1] = 0;
						
						if (print)
						{
							/* hide cursor */
							__terminalWrite(term, __terminalHideCursor);
							
							__terminalWrite(term, term->rx_line + term->cursor - 1);
							
							/* move cursor back */
							__terminalBack(term, term->rxoffs - term->cursor);
							
							/* show cursor */
							__terminalWrite(term, __terminalShowCursor);
							echo = __FALSE;
						}
					} else
					{
						echo = __FALSE;
					}
				}
				
				/* Has to send back ECHO? */
				if (echo && (term->flags & __TERMINAL_ECHO_ENABLED))
				{
					__terminalWriteChar(term, c);
				}
				
				/* End of line, return */
				if (ret)
				{
					term->rxoffs = 0;
					term->cursor = 0;
					return __TRUE;
				}
			}
		}

		__threadSleep(1);
	}

	return __FALSE;
}

/*!
 * @brief Terminal thread.
 *
 * This function will read from the device and call registered threads callback functions.
 * @return	Nothing.
 *
 */
__VOID __terminalThread(__VOID)
{
	__PTERMINALCMD commands;
	__PSTRING 	str;
	__BOOL		bKnown = __FALSE;
	__PTERMINAL term = __threadGetParameter();

	__terminalWriteLine(term, "\r\n\r\n");
	
	__terminalWriteLine(term, "\r\nMilos v3 - Terminal Ready\r\n");
	__terminalShowPrompt(term);
	
	for (;;)
	{
		__memSet(term->rx_line, 0, term->linelen);
		if (__terminalReadLine(term))
		{
			if (__strLen(term->rx_line))
			{
				/* Cycle through all terminal commands */
				commands = term->cmds;
				bKnown = __FALSE;
				
				/* Command with parameters? */
				str = __strChr(term->rx_line, 0x20);

				while (commands && !bKnown)
				{
					if (str)
					{
						if (__strnCmp(term->rx_line, commands->cmd, (u16) (str - term->rx_line)) == 0)
						{
							if (commands->func) (commands->func) (term, term->rx_line, str);
							bKnown = __TRUE;
						}

					} else {
						/* No parameters, command must exactly match */
						if (__strCmp(term->rx_line, commands->cmd) == 0)
						{
							if (commands->func) (commands->func) (term, term->rx_line, __NULL);
							bKnown = __TRUE;
						}
					}

					commands = commands->next;
				}

				/* Unknown Command? */
				if (bKnown == __FALSE)
				{
					__terminalWriteLine(term, "Unknown Command\r\n");
				}
			}
			
			__terminalShowPrompt(term);
		}
		
		__threadSleep(1);
	}
}

/*!
 * @brief Terminal initialization.
 *
 * Call this function passing a pointer to a device to active the terminal.
  * @param	term	Pointer to the terminal to be initialized.
 * @return	Nothing.
 *
 */
__BOOL __terminalStart(__PTERMINAL term)
{
	/* Already started? */
	if (term->flags & __TERMINAL_STARTED) return __FALSE;
	
	/* Check for at least one device */
	if (!term->dv_in && !term->dv_out) return __FALSE;

	/* Check for devices opened and initialized */
	if (term->dv_in)
	{
		if (!term->dv_in->dv_initd || !term->dv_in->dv_opcnt) return __FALSE;
	}

	if (term->dv_out)
	{
		if (!term->dv_out->dv_initd || !term->dv_out->dv_opcnt) return __FALSE;
	}

	term->flags |= __TERMINAL_STARTED;

	/* Is the device has not an input device, do not start the terminal "read" thread */
	if (term->dv_in)
	{
		__threadCreate(term->name, __terminalThread, __CONFIG_PRIO_TERMTHREAD, 1024, 1, term);
	}

	return __TRUE;
}

/*!
 * @brief Outputs a line of text.
 *
 * Writes a line of text, using __deviceWriteLine() function (automatically adds CR and/or LF).
 * Accepts sprintf()-like arguments.
 *
 * @param	term		Pointer to a __TERMINAL structure.
 * @param	str			String of text.
 * @param	...			sprintf()-like arguments.
 *
 * @return	Nothing.
 *
 */
__VOID __terminalWriteLine(__PTERMINAL term, __CONST __PSTRING str, ...)
{
	__PSTRING args = ((__PSTRING) &str) + sizeof(__PSTRING);
	__terminalOut(term, __TRUE, str, args);
}

/*!
 * @brief Outputs a line of text.
 *
 * Writes a line of text, using __deviceWrite() function.
 * Accepts sprintf()-like arguments.
 *
 * @param	term		Pointer to a __TERMINAL structure.
 * @param	str			String of text.
 * @param	...			sprintf()-like format.
 *
 * @return	Nothing.
 *
 */
__VOID __terminalWrite(__PTERMINAL term, __CONST __PSTRING str, ...)
{
	__PSTRING args = ((__PSTRING) &str) + sizeof(__PSTRING);
	__terminalOut(term, __FALSE, str, args);
}

__VOID __terminalOut(__PTERMINAL term, __BOOL crlf, __CONST __PSTRING str, __PSTRING args)
{
	if (!(term->flags & __TERMINAL_STARTED)) return;

	if (__strLen(str) >= term->linelen) return;

	if (__lockOwn(term->lock, 1000))
	{
		if (args)
		{
			__strFmtEx(term->tx_line, str, args);
		} else {
			__strCpy(term->tx_line, str);
		}

		__deviceWrite(term->dv_out, term->tx_line, __strLen(term->tx_line));

		if (crlf)
		{
			__deviceWrite(term->dv_out, "\r\n", 2);
		}

		__deviceFlush(term->dv_out);
		__lockRelease(term->lock);
	}
}


/*!
 * @brief Creates a terminal.
 *
 * Call this function to create a terminal. At least one device (input or output) is required.
 * The devices are not required to be initialized or opened before calling this function.
 *
 * Although, a terminal can be directly started with __terminalStart() by filling the required
 * fields (the same described here below) of the __PTERMINAL term parameter.
 *
 * @param	name		Null-terminated string indicating the terminal name.
 * @param	in			Input device.
 * @param	out			Output device.
 * @param 	linelen		Maximum line length.
 * @param	prompt		Custom (null-terminated) command prompt.
 * @param	flags		Creation flags.
 * @arg		__TERMINAL_ECHO_ENABLED 	Enables the terminal output echo.
 * @arg		__TERMINAL_DEBUG_COMMANDS 	Enables debug commands as "heap", "threads" and others.
 * 										Available only with __CONFIG_COMPILE_DBGTERM = 1.
 *
 * @return	On success, a pointer to the newly created terminal. __NULL on failure.
 *
 */
__PTERMINAL __terminalCreate(__PSTRING name, __PDEVICE in, __PDEVICE out, u16 linelen, __PSTRING prompt, u8 flags)
{
	__PTERMINAL term;

	if (!name) return __NULL;

	/* Check for at least one device */
	if (!in && !out) return __NULL;

	/* Create terminal */
	term = __heapAllocZero(sizeof(__TERMINAL));
	if (!term) return __NULL;

	/* Check line length */
	if (!linelen) linelen = __TERMINAL_DEF_LINELEN;

	/* Allocate RX line */
	term->rx_line = __heapAllocZero(linelen);
	if (!term->rx_line)
	{
		__heapFree(term);
		return __NULL;
	}

	/* Allocate TX line */
	term->tx_line = __heapAllocZero(linelen);
	if (!term->tx_line)
	{
		__heapFree(term);
		__heapFree(term->rx_line);
		return __NULL;
	}

	/* If an output device is provided, create a lock
	 * for writing */
	if (out)
	{
		term->lock = __lockCreate();
		if (!term->lock)
		{
			__heapFree(term);
			__heapFree(term->rx_line);
			__heapFree(term->tx_line);
			return __NULL;
		}
	}

	term->linelen = linelen;
	term->name = name;
	term->dv_in = in;
	term->dv_out = out;
	term->flags = flags;

	if (!prompt)
	{
		term->prompt = __TERMINAL_PROMPT;
	} else {
		term->prompt = prompt;
	}

#if __CONFIG_COMPILE_DBGTERM

	if (flags & __TERMINAL_DEBUG_COMMANDS)
	{
		__terminalAddCommand(term, __dbgCommands, sizeof(__dbgCommands) / sizeof(__TERMINALCMD));
	}

#endif /* __CONFIG_COMPILE_DBGTERM */

	return term;
}

/*!
 * @brief Adds a command to the list of commands of a terminal.
 *
 * Call this function providing a __TERMINALCMD structure containing a null-terminated
 * string in the \c cmd member specifying the command to be processed and a pointer
 * to a __TERMINALRXCMD function type (\c func member), to be called when the \c cmd command
 * is received. The \c next member of __TERMINALCMD structure the can be ignored.
 *
 * After calling this function, each time the specified command
 * is typed (received) through the terminal, the function \c func
 * will be called.
 *
 * Different commands can be registered in a single __terminalAddCommand() call.
 *
 * @param	term 	Pointer to a terminal.
 * @param 	cmd		Pointer to a list of __TERMINALCMD structures.
 * @param	count	Quantity of __TERMINALCMD structures in the list.
 *
 * @return	Nothing.
 *
 */
__VOID __terminalAddCommand(__PTERMINAL term, __PTERMINALCMD cmd, u8 count)
{
	u8 i;

	if (!term || !cmd || !count) return;

	for (i = 0; i < count; i++)
	{
		cmd[i].next = term->cmds;
		term->cmds = &cmd[i];
	}
}

/*!
 * @brief Creates and adds a command to the terminal command list.
 *
 * Call this function to create and add a command to the terminal command list.
 * Each time the \c cmd command is detected, the function \c func will be called.
 *
 * @param term	Pointer to a terminal.
 * @param cmd	Pointer to a null-terminated string representing the command.
 * @param func	Function to be called when the \c cmd command is received.
 *
 * @return Nothing.
 */
__VOID __terminalCreateCommand(__PTERMINAL term, __PSTRING cmd, __TERMINALRXCMD* func)
{
	__PTERMINALCMD ptr;

	if (!term) return;

	ptr = __heapAllocZero(sizeof(__TERMINALCMD));
	if (!ptr) return;

	ptr->cmd = cmd;
	ptr->func = func;
	ptr->next = __NULL;

	__terminalAddCommand(term, ptr, 1);
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

#endif // __CONFIG_COMPILE_TERMINAL

