/***************************************************************************
 * plat_ostypes.h
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

#ifndef	__PLAT_OSTYPES_H__
#define	__PLAT_OSTYPES_H__

// Include ST library.
// Will also include default typedefs so if is not defined STM32F10x
// some Milos types will not be defined below.
#include <stm32f4xx.h>

#ifdef __GNUC__
#define __TYPEDEF_PRE
#define __TYPEDEF_POST	__attribute__ ((__packed__))
#else
#define __TYPEDEF_PRE	__packed
#define __TYPEDEF_POST
#endif

//	Internal types
typedef	char					i8;			/*!< @brief 8 bits signed char */
typedef	short					i16;		/*!< @brief 16 bits signed int */
typedef	signed int				i32;		/*!< @brief 32 bits signed int */
typedef	unsigned char			__BOOL;		/*!< @brief Boolean for function return */
typedef float					flt32;		/*!< @brief 32 bits float value */
typedef double					flt64;		/*!< @brief 64 bits float value */

#define __VOID					void		/*!< @brief Void value */

typedef union {								/*!< @brief 32 bits float */
	
	flt32		flt;
	u8			bytes[4];
	u32			ul;
	
}__FLT32;

typedef union {								/*!< @brief 64 bits float */
	
	flt64		f;
	u8			bytes[8];
	
} __FLT64;

//	Internal pointer types
#define	pi8				i8*
#define	pu8				u8*
#define	pi16			i16*
#define	pu16			u16*
#define	pi32			i32*
#define	pu32			u32*
#define	__PVOID			__VOID*

//	Other
#define	__FAR			_far
#define	__CONST			const

#ifdef STRING_IS_UNSIGNED
#define __STRING		u8
#else
#define __STRING		i8
#endif // STRING_IS_UNSIGNED

#define	__PSTRING		__STRING*


#define	__TRUE			1
#define	__FALSE			!__TRUE
#define	__NULL			(__PVOID) 0

#define __VOLATILE		volatile


//	Structure to define a 8 bits value

typedef union __def8Tag {

	u8			byte;					// Full u8 access

	struct {							// Access by nibbles

		u8		nib0:4;					// Low nibble
		u8		nib1:4;					// High nibble

	} nibbles;

	struct {							// Access by pairs

		u8		pair0:2;				// 1st pair
		u8		pair1:2;				// 2nd pair
		u8		pair2:2;				// 3rd pair
		u8		pair3:2;				// 4th pair

	} pairs;

	struct {							// Access by bits

		u8		bit0:1;					// Bit 0
		u8		bit1:1;					// Bit 1
		u8		bit2:1;					// Bit 2
		u8 		bit3:1;					// Bit 3
		u8 		bit4:1;					// Bit 4
		u8 		bit5:1;					// Bit 5
		u8 		bit6:1;					// Bit 6
		u8 		bit7:1;					// Bit 7

	} bits;

} __DEF_8;

//	Structure to define a 16 bits value

typedef union {

	u16			uval;				// Full access to u16
	i16			ival;				// Full access to signed u16

	struct {						// Access to u8

		u8		low;				// Low u8
		u8		high;				// High u8

	} bytes;

	struct {						// Access to nibbles

		u8		nib0:4;				// Nibble 0
		u8		nib1:4;				// Nibble 1
		u8		nib2:4;				// Nibble 2
		u8		nib3:4;				// Nibble 3

	} nibbles;

	struct {						// Access by pairs

		u8		pair0:2;			// 1st pair
		u8		pair1:2;			// 2nd pair
		u8		pair2:2;			// 3rd pair
		u8		pair3:2;			// 4th pair
		u8		pair4:2;			// 5th pair
		u8		pair5:2;			// 6th pair
		u8		pair6:2;			// 7th pair
		u8		pair7:2;			// 8th pair

	} pairs;

	struct {  						// Access to bits

		u8 		bit0:1;				// Bit 0
		u8 		bit1:1;				// Bit 1
		u8 		bit2:1;				// Bit 2
		u8 		bit3:1;				// Bit 3
		u8 		bit4:1;				// Bit 4
		u8 		bit5:1;				// Bit 5
		u8 		bit6:1;				// Bit 6
		u8 		bit7:1;				// Bit 7
		u8 		bit8:1;				// Bit 8
		u8 		bit9:1;				// Bit 9
		u8 		bit10:1;			// Bit 10
		u8 		bit11:1;			// Bit 11
		u8 		bit12:1;			// Bit 12
		u8 		bit13:1;			// Bit 13
		u8 		bit14:1;			// Bit 14
		u8 		bit15:1;			// Bit 15

	} bits;

} __DEF_16;

typedef union {

	u32			uval;				// Full access to u16
	i32			ival;				// Full access to signed u16

	struct {						// Access to u8

		u16		low;				// Low u8
		u16		high;				// High u8

	} words;

	struct {						// Access to nibbles

		u8		byte0:4;			// Nibble 0
		u8		byte1:4;			// Nibble 1
		u8		byte2:4;			// Nibble 2
		u8		byte3:4;			// Nibble 3

	} bytes;


	struct {  						// Access to bits

		u8 		bit0:1;				// Bit 0
		u8 		bit1:1;				// Bit 1
		u8 		bit2:1;				// Bit 2
		u8 		bit3:1;				// Bit 3
		u8 		bit4:1;				// Bit 4
		u8 		bit5:1;				// Bit 5
		u8 		bit6:1;				// Bit 6
		u8 		bit7:1;				// Bit 7
		u8 		bit8:1;				// Bit 8
		u8 		bit9:1;				// Bit 9
		u8 		bit10:1;			// Bit 10
		u8 		bit11:1;			// Bit 11
		u8 		bit12:1;			// Bit 12
		u8 		bit13:1;			// Bit 13
		u8 		bit14:1;			// Bit 14
		u8 		bit15:1;			// Bit 15
		u8 		bit16:1;			// Bit 15
		u8 		bit17:1;			// Bit 15
		u8 		bit18:1;			// Bit 15
		u8 		bit19:1;			// Bit 15
		u8 		bit20:1;			// Bit 15
		u8 		bit21:1;			// Bit 15
		u8 		bit22:1;			// Bit 15
		u8 		bit23:1;			// Bit 15
		u8 		bit24:1;			// Bit 15
		u8 		bit25:1;			// Bit 15
		u8 		bit26:1;			// Bit 15
		u8 		bit27:1;			// Bit 15
		u8 		bit28:1;			// Bit 15
		u8 		bit29:1;			// Bit 15
		u8 		bit30:1;			// Bit 15
		u8 		bit31:1;			// Bit 15

	} bits;

} __DEF_32;

#ifdef	__DEBUG
#define	__STATIC
#else
#define	__STATIC	static
#endif

#endif /* __PLAT_OSTYPES_H__ */
