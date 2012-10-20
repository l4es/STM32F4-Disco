/***************************************************************************
 * mem.h
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

#ifndef __MEM_H__
#define __MEM_H__

#include <plat_ostypes.h>

/** @addtogroup Common
  * @{
  */

/** @defgroup Memory Memory
  * @{
  */


__VOID	__memSet(__PVOID ptr, u8 fill, u16 qty);
__VOID	__memCpy(__PVOID dst, __CONST __PVOID src, u16 qty);
u8 		__memCmp(__CONST __PVOID ptr1, __CONST __PVOID ptr2, u16 qty);

/**
  * @}
  */

/**
  * @}
  */

#endif /* __MEM_H__ */
