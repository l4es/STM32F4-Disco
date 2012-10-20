/***************************************************************************
 * dbgterm.h
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

#ifndef __DBGTERM_H__
#define __DBGTERM_H__

#include <plat_cpu.h>
#include "terminal.h"

#if __CONFIG_COMPILE_DBGTERM

/** @addtogroup DbgTerminal
  * @{
  */

__VOID __dbgInit(__VOID);
__VOID __dbgWrite(__CONST __PSTRING str, ...);
__VOID __dbgWriteLine(__CONST __PSTRING str, ...);
__VOID __dbgAddCommand(__PSTRING cmd, __TERMINALRXCMD* func);
__PTERMINAL __dbgGetSysTermPtr(__VOID);
__VOID __dbgTerminalIn(__PTERMINAL term, __PSTRING str, __PSTRING params);

/*! @brief Terminal output with CR/LF */
#define DBGMSG(cond, exp) if (cond) __dbgWriteLine exp
/*! @brief Terminal output without CR/LF */
#define DBGOUT(cond, exp) if (cond) __dbgWrite exp

#else
/* define terminal macros */
#define DBGMSG(cond, exp)
#define DBGOUT(cond, exp)

#endif /* __CONFIG_DBGTERM_ENABLED */

/**
  * @}
  */

#endif /* __DBGTERM_H__ */
