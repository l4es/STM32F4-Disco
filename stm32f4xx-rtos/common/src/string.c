/***************************************************************************
 * string.c
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


#include "string.h"
#include "mem.h"

#if __CONFIG_COMPILE_STRING

/** @addtogroup String
  * @{
  */

__STATIC __CONST u8 __hexFmt[] = "0123456789abcdef";

#define	__FMT_SIGN		0x0001
#define	__FMT_LONG		0x0002

/*!
 * @brief Converts string to upper case
 * @param ptr	Pointer to string to convert.
 * @return Pointer to the string.
 *
 */
__PSTRING 	__strUpper(__PSTRING ptr)
{
	u16		i;
	u16		len = __strLen(ptr);

	for (i = 0; i < len; i++)
		*(ptr + i) = __toUpper(*(ptr + i));

	return(ptr);
}

/*!
 * @brief Converts char to upper case
 * @param c	Char to convert.
 * @return Converted char.
 *
 */
u8 __toUpper(__CONST u8 c)
{

	if (c >= 'a' && c <= 'z')
	{
		return(c & 0xDF);
	} else {
		return(c);
	}
}

/*!
 * @brief Converts char to lower case
 * @param c	Char to convert.
 * @return Converted char.
 *
 */
u8 __toLower(__CONST u8 c)
{

	if (c >= 'A' && c <= 'Z')
	{
		return(c | 0x20);
	} else {
		return(c);
	}
}

/*!
 * @brief Converts string to lower case
 * @param ptr	Pointer to string to convert.
 * @return Pointer to the string.
 *
 */
__PSTRING __strLower(__PSTRING ptr)
{
	u16		i;
	u16		len = __strLen(ptr);

	for (i = 0; i < len; i++)
		*(ptr + i) = __toLower(*(ptr + i));

	return(ptr);
}


/*!
 * @brief Compares two strings. Not case-sensitive.
 * @param str1	String 1 to compare.
 * @param str2	String 2 to compare.
 * @return -1 if str is less than str2. 0 if str1 is identical to str2. 1 if str1 is
 * greater than str2.
 *
 */
i16 __strCmpNoCase(__CONST __PSTRING str1, __CONST __PSTRING str2)
{
	u8		c, d;

	while (__TRUE)
	{
		c = __toUpper(*str1++);
		d = __toUpper(*str2++);
		if (c == 0 && d != 0) return(-1);
		if (d == 0 && c != 0) return(1);
		if (c == 0 && d == 0) return(0);
		if (c < d) return(-1);
		if (c > d) return(1);
	}

	return (0);
}

/*!
 * @brief Compares two strings. Case-sensitive.
 * @param str1	String 1 to compare.
 * @param str2	String 2 to compare.
 * @return -1 if str is less than str2. 0 if str1 is identical to str2. 1 if str1 is
 * greater than str2.
 *
 */
i16	__strCmp(__CONST __PSTRING str1, __CONST __PSTRING str2)
{
	u8		c, d;

	while (__TRUE)
	{
		c = *str1++;
		d = *str2++;
		if (c == 0 && d > 0) return(-1);
		if (d == 0 && c > 0) return(1);
		if (c == 0 && d == 0) return(0);
		if (c < d) return(-1);
		if (c > d) return(1);
	}

	return (0);
}

/*!
 * @brief Gets the string lenght.
 * @param str Pointer to a string.
 * @return The string lenght in bytes.
 */
u16	__strLen(__CONST __PSTRING str)
{
	u16		len = 0;

	while (*str++ != 0) ++len;
	return(len);
}

/*!
 * @brief Tests for a character in a string.
 * @param src Pointer to a string.
 * @param chr Character to find.
 * @return Pointer to the found character inside the string. If not found it
 * will return __NULL;
 *
 */
__PSTRING __strChr(__CONST __PSTRING src, u8 chr)
{
	u16 i = 0;

	while (src[i] > 0)
	{
		if (src[i] == chr) return((__PSTRING) &src[i]);
		++i;
	}
	return(__NULL);
}

/*!
 * @brief Copies \c src string into \c dst string, appending a final 0.
 * @param dst Pointer to destination string.
 * @param src Pointer to the source string.
 * @return Pointer to the destination string.
 *
 */
__PSTRING	__strCpy(__PSTRING dst, __CONST __PSTRING src)
{

	__PSTRING	ret = dst;

	while (*src != 0) *dst++ = *src++;
	*dst = 0;
	return(ret);
}

/*!
 * @brief Appends \c src string to \c dst string.
 * @param dst Pointer to destination string.
 * @param src Pointer to the source string.
 * @return Pointer to the destination string.
 *
 */
__PSTRING	__strCat(__PSTRING dst, __CONST __PSTRING src)
{

	__PSTRING	ret = dst;

	dst += __strLen(dst);
	while (*src != 0) *dst++ = *src++;
	*dst = 0;
	return(ret);
}

/*!
 * @brief Converts an u32 value to a string.
 * @param buf Pointer to string receiving the converted value.
 * @param val Value to convert.
 * @param width Blank spaces left to align.
 * @return Converted string value length.
 *
 */
i16	__u32toStr(__PSTRING buf, u32 val, i16 width)
{
	__STRING st[24];
	u8		va = 23;
	i16		dg;

	st[va--] = 0;
	if (val == (u32) 0) {
		st[va--] = '0';
		width--;
	} else {
		while (val > 0) {
			dg = (i16) (val % 10) + '0';
			val /= 10;
			st[va--] = (u8) dg;
			--width;
		}
	}
	while (va > 0 && width-- > 0) st[va--] = ' ';
	__strCpy(buf,st + va + 1);
	return(__strLen(buf));
}

/*!
 * @brief Converts an u16 value to a string.
 * @param buf Pointer to string receiving the converted value.
 * @param val Value to convert.
 * @param width Blank spaces left to align.
 * @return Converted string value lenght.
 *
 */
i16	__u16toStr(__PSTRING buf, u16 val, i16 width)
{
	return(__u32toStr(buf,(u32) val,width));
}

/*!
 * @brief Converts an i32 value to a string.
 * @param buf Pointer to string receiving the converted value.
 * @param val Value to convert.
 * @param width Blank spaces left to align.
 * @return Converted string value lenght.
 *
 */
i16	__i32toStr(__PSTRING buf, i32 val, i16 width)
{

	__STRING st[24];
	u8		va = 23;
	u8		sg = 0;
	i16		dg;

	st[va--] = 0;
	if (val < 0)
	{
		val = -val;
		sg = 1;
	}

	if (val == 0)
	{
		st[va] = '0';
	}
	else
	{
		while (val > 0)
		{
			dg = (i16) (val % 10) + '0';
			val /= 10;
			st[va--] = (u8) dg;
			--width;
		}
	}

	if (sg)
	{
		st[va--] = '-';
		--width;
	}

	while (va > 0 && width-- > 0) st[va--] = ' ';
	__strCpy(buf,st + va + 1);
	return(__strLen(buf));
}

/*!
 * @brief Converts an i16 value to a string.
 * @param buf Pointer to string receiving the converted value.
 * @param val Value to convert.
 * @param width Blank spaces left to align.
 * @return Converted string value lenght.
 *
 */
i16	__i16toStr(__PSTRING buf, i16 val, i16 width)
{
	return(__i32toStr(buf,(i32) val,width));
}

/*!
 * @brief Fills 0x20 value in a string with \c c character.
 *
 * Internal function. Do not call.
 * @return Nothing.
 *
 */
__STATIC __VOID __strFill(__PSTRING ptr, u8 c)
{
	while(*ptr == ' ') *ptr++ = c;
}

/*!
 * @brief __strFmt() function helper.
 *
 * Internal function. Do not call.
 * @return Parsed string.
 *
 */
__STATIC __PSTRING __strPrn(__PSTRING *ptr, __PSTRING fmt, __PSTRING *arg, pi16 qty, pi16 ifm, pi16 ipp)
{
	u16		flg = 0;
	pu8		*pp = (pu8*) *arg;
	u8		pad = ' ';
	i16		wdt = 0;
	i16		len;
	i16		n, i;
	i16		pos;
	u32		val;
	pu32	pvu32 = (pu32) *arg;
	pu16	pvu16 = (pu16) *arg;
	pu8		pvu8;
	u8		c, d;
	__STRING str[33];

	str[0] = 0;
	if (*fmt == '-')
	{
		flg |= __FMT_SIGN;
		++(*ifm);
		++fmt;
	}

	if (*fmt >= '0' && *fmt <= '9')
	{
		if (*fmt == '0')
		{
			pad = '0';
			++fmt;
			++(*ifm);
		}

		while (*fmt >= '0' && *fmt <= '9')
		{
			wdt = (wdt * 10) + (*fmt++ & 0x0F);
			++(*ifm);
		}
	}

	if (*fmt == 'l')
	{
		flg |= __FMT_LONG;
		++fmt;
		++(*ifm);
	}

	if (__strChr((__PSTRING) "csiduxX",*fmt) != __NULL)
	{
		++(*ifm);
		c = *fmt++;
		switch(c)
		{
			case 'c':	val = (u32) *pvu16;
						**ptr = (u8) val;
						++*ptr;
						*ipp += 2;
						*qty += wdt;
						break;
			case 'x':
			case 'X':	if (flg & __FMT_LONG)
						{
							val = *pvu32;
							n = 8;
						} else {
							val = (u32) *pvu16;
							n = 4;
							*ipp -= 2;		/* Use +4 from string */
						}

						pos = sizeof(str) - 1;
						str[pos--] = 0;
						for (i = 0; i < n; i++)
						{
							d = (u8) (val & 0x0000000F);
							val = val >> 4;
							if (i > 0 && d == 0 && val == 0 && pad == ' ')
							{
								pvu8 = (u8*) &str[pos + 1];
								if (c == 'X') __strUpper((__PSTRING) pvu8);
								goto vstr;
							}
							str[pos--] = __hexFmt[d];
						}
						pvu8 = (u8*) &str[pos + 1];
						if (c == 'X') __strUpper((__PSTRING) pvu8);
						goto vstr;

			case 'i':
			case 'd':
			case 'u':
				if (flg & __FMT_LONG)
				{
					if (wdt >= 0 && wdt <= 20) {
						if (c == 'u')
						{
							__u32toStr(str,*pvu32,wdt);
						} else {
							__i32toStr(str,*pvu32,wdt);
						}
					}
					*ipp += 4;
				} else {
					if (wdt >= 0 && wdt <= 20)
					{
						if (c == 'u')
						{
							__u16toStr(str,*pvu16,wdt);
						} else {
							__i16toStr(str,*pvu16,wdt);
						}
					}
					*ipp += 2;
				}

				if (pad == '0')
				{
					__strFill(str,'0');
				}
				__strCpy(*ptr,str);
				wdt = __strLen(str);
				*qty += wdt;
				*ptr += wdt;
				break;

			case 's':
				pvu8 = *pp;
vstr:
				len = __strLen((__PSTRING) pvu8);
				if (len > wdt)
				{
					__strCpy(*ptr, (__PSTRING) pvu8);
					*qty += len;
					*ptr += len;
				} else {
					if (flg & __FMT_SIGN)
					{
						__memSet(*ptr,' ',wdt - len);
						*ptr += (wdt - len);
						__strCpy(*ptr, (__PSTRING) pvu8);
						*ptr += len;
					} else {
						__strCpy(*ptr, (__PSTRING) pvu8);
						*ptr += len;
						__memSet(*ptr,' ',wdt - len);
						*ptr += (wdt - len);
					}
					*qty += wdt;
				}
				*ipp = 4;
				break;
		}
	} else {
		++fmt;
	}
	return(fmt);
}

/*!
 * @brief	Formats string. This function is not intended to fully replace the
 *			standard sprintf function, but is lightweight and usable.
 * @param	dst Destination buffer.
 * @param	fmt Format string %[FLAGS][WIDTH][MOD]TYPE
 *
 * FLAGS:
 *			If string and WIDTH, right justification
 *			%20s	Left justification
 *			%-20s	Right justification
 *			Ex: __strFmt(str,"%8s","ABS");
 *			str = "ABS     "
 *			Ex: __strFmt(str,"%-8s","ABS");
 *			str = "     ABS"
 *
 * WIDTH:
 *			Result width. If start with zero, will be filled.
 *			Ex: __strFmt(str,"%8lu",(u32) 128);
 *			str = "     128"
 *			Ex: __strFmt(str,"%08lu",(u32) 128);
 *			str = "00000128"
 *
 * MOD:
 *			l	Use long value
 *
 * TYPE:
 *			d	Signed integer
 *			i	Signed integer
 *			u	Unsigned integer
 *			s	String
 *			c	Char
 *			x	Hexadecimal lowercase
 *			X	Hexadecimal uppercase
 *
 * @return	Destination string length.
 *
 */
i16	__strFmt(__PSTRING dst, const __PSTRING fmt, ...)
{
	__PSTRING arg = ((__PSTRING) &fmt) + sizeof(__PSTRING);
	__PSTRING ptr = dst;
	__PSTRING fm = (__PSTRING) fmt;
	i16	qty = 0;
	i16	ifm;
	i16	ipp;

	while(*fm)
	{
		if (*fm == '%')
		{
			++fm;
			if (*fm == '%') {
				*ptr++ = '%';
				++fm;
				++qty;
			} else {
				ifm = ipp = 0;
				__strPrn(&ptr,fm,&arg,&qty,&ifm,&ipp);
				fm += ifm;
				arg += ipp;
			}
		} else {
			*ptr++ = *fm++;
			++qty;
		}
	}
	*ptr = 0;
	return(qty);
}

/*!
 * @brief Same as __strFmt(), but does not uses stack based arguments.
 *
 * @return Destination string length.
 *
 */
i16	__strFmtEx(__PSTRING dst, __CONST __PSTRING fmt, __PSTRING args)
{

	__PSTRING arg = args;
	__PSTRING ptr = dst;
	__PSTRING fm = (__PSTRING) fmt;
	i16 qty = 0;
	i16	ifm;
	i16	ipp;

	while(*fm)
	{
		if (*fm == '%')
		{
			++fm;
			if (*fm == '%')
			{
				*ptr++ = '%';
				++fm;
				++qty;
			} else {
				ifm = ipp = 0;
				__strPrn(&ptr,fm,&arg,&qty,&ifm,&ipp);
				fm += ifm;
				arg += ipp;
			}
		} else {
			*ptr++ = *fm++;
			++qty;
		}
	}
	*ptr = 0;
	return(qty);
}

/*!
 * @brief Compares the first 'len' characters of two strings. Not case-sensitive
 * @param		str1		String 1 to compare.
 * @param		str2		String 2 to compare.
 * @param		len			Number of characters to compare.
 * @return		-1 if str1 is less than str2. 0 if str1 is identical to str2.
 *				1 if str1 is greater than str2.
 *
 */
i16 __strnCmpNoCase(__CONST __PSTRING str1, __CONST __PSTRING str2, u16 len)
{

	u8		c, d;
	if (len > 0)
	{
		while (__TRUE)
		{
			c = __toUpper(*str1++);
			d = __toUpper(*str2++);
			if (c == 0 && d > 0) return(-1);
			if (d == 0 && c > 0) return(1);
			if (c == 0 && d == 0) return(0);
			if (c < d) return(-1);
			if (c > d) return(1);
			if (--len == 0) return(0);
		}
	}
	return(0);
}

/*!
 * @brief Compares the first 'len' characters of two strings. Case sensitive
 * @param		str1		String 1 to compare.
 * @param		str2		String 2 to compare.
 * @param		len			Number of characters to compare.
 * @return		-1 if str1 is less than str2. 0 if str1 is identical to str2.
 *				1 if str1 is greater than str2.
 *
 */
i16 __strnCmp(__CONST __PSTRING str1, __CONST __PSTRING str2, u16 len)
{

	u8		c, d;
	if (len > 0)
	{
		while (__TRUE)
		{
			c = *str1++;
			d = *str2++;
			if (c == 0 && d > 0) return(-1);
			if (d == 0 && c > 0) return(1);
			if (c == 0 && d == 0) return(0);
			if (c < d) return(-1);
			if (c > d) return(1);
			if (--len == 0) return(0);
		}
	}
	return(0);
}

i32 __strToI32(__CONST __PSTRING p) {
i32 n;
i32 f;

	n = 0;
	f = 0;
	for(;;p++) {
		switch(*p) {
		case ' ':
		case '\t':
			continue;
		case '-':
			f++;
			/* no break */
		case '+':
			p++;
			/* no break */
		}
		break;
	}
	while(*p >= '0' && *p <= '9')
		n = n*10 + *p++ - '0';
	return(f? -n: n);
}

u32 __strToU32(__CONST __PSTRING str) {
    u32 res = 0;
    i32 i ;
    for(i = 0; (str[i] >='0' && str[i] <='9') || str[i] == ' '; ++i)
    {
    	if (str[i] != ' ') {
    		res = 10 * res + str[i] - '0';
    	}
    }
    return res;
}

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_STRING */

