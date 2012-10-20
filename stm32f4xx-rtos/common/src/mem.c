/***************************************************************************
 * mem.c
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

#include "mem.h"

/** @addtogroup Memory
  * @{
  */

/*!
 * @brief Fills memory with char.
 *
 * @param ptr	Pointer to memory to be filled.
 * @param fill	Value to fill into.
 * @param qty	Quantity of bytes to fill.
 * @return		Nothing.
 *
 */
__VOID 	__memSet(__PVOID ptr, u8 fill, u16 qty)
{
	pu8		p = ptr;

	if (qty > 0)
	{
		while (qty-- > 0)
			*p++ = fill;
	}
}

/*!
 * @brief Copies a memory block.
 *
 * @param dst	Pointer to destination.
 * @param src	Pointer to source.
 * @param qty	Quantity of bytes to copy.
 * @return		Nothing.
 *
 */
__VOID __memCpy(__PVOID dst, __CONST __PVOID src, u16 qty)
{
	__PSTRING	ds = (__PSTRING) dst;
	__PSTRING	sr = (__PSTRING) src;

	while (qty-- > 0) *ds++ = *sr++;
}

/*!
 * @brief Compares two memory blocks.
 *
 * @param ptr1	Pointer to the first memory block.
 * @param ptr2	Pointer to the second memory block.
 * @param qty	Quantity of bytes to compare.
 * @return		Zero if the blocks are equal, otherwise nonzero.
 *
 */
u8 __memCmp(__CONST __PVOID ptr1, __CONST __PVOID ptr2, u16 qty)
{
	pu8		p1 = (pu8) ptr1;
	pu8		p2 = (pu8) ptr2;

	while (qty-- > 0)
		if (*p1++ != *p2++) return(1);

	return(0);
}

/**
  * @}
  */

