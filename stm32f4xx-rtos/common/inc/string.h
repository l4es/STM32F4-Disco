/***************************************************************************
 * @file	string.h
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

#ifndef __STRING_H__
#define __STRING_H__

#include <plat_cpu.h>

#define __u8ToStr(x) ((x<=9) ? x+0x30 : x+0x37)

/** @addtogroup Common
  * @{
  */

/** @defgroup String String
  * @{
  */

__PSTRING 	__strLower(__PSTRING ptr);
u8			__toLower(__CONST u8 c);
u8			__toUpper(__CONST u8 c);
__PSTRING 	__strUpper(__PSTRING ptr);
i16			__strCmpNoCase(__CONST __PSTRING str1, __CONST __PSTRING str2);
i16 		__strnCmp(__CONST __PSTRING str1, __CONST __PSTRING str2, u16 len);
i16			__strCmp(__CONST __PSTRING str1, __CONST __PSTRING str2);
u16			__strLen(__CONST __PSTRING str);
__PSTRING	__strChr(__CONST __PSTRING src, u8 chr);
__PSTRING	__strCpy(__PSTRING dst, __CONST __PSTRING src);
__PSTRING	__strCat(__PSTRING dst, __CONST __PSTRING src);
i16			__strFmt(__PSTRING dst, __CONST __PSTRING fmt, ...);
i16			__strnCmpNoCase(__CONST __PSTRING str1, __CONST __PSTRING str2, u16 len);
i16			__strFmtEx(__PSTRING dst, __CONST __PSTRING fmt, __PSTRING args);
i32 		__strToI32(__CONST __PSTRING p);
u32 		__strToU32(__CONST __PSTRING str);

/**
  * @}
  */

/**
  * @}
  */

#endif /* __STRING_H__ */
