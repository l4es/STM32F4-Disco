/***************************************************************************
 * intvect.c
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

#include "intrvect.h"
#include <plat_cpu.h>

__INTFUNC		*__fnPtr[__PLATCONFIG_MAX_IRQ];		/*!< @brief Interrupt vector table */
__PVOID			__arg[__PLATCONFIG_MAX_IRQ];		/*!< @brief Argument table */

#if __PLATCONFIG_GET_IRQ_SOURCE

__VOID __intArrival(__VOID)
{
	u32 irq;

	if (__cpuGetInterruptSource(&irq) && irq < __PLATCONFIG_MAX_IRQ)
	{
		if (__fnPtr[irq] != __NULL) __fnPtr[irq](__arg[irq]);
	}
}

#else

__VOID	__intVect_00(__VOID) {

	if (__fnPtr[0] != __NULL) __fnPtr[0](__arg[0]);
}

__VOID	__intVect_01(__VOID) {

	if (__fnPtr[1] != __NULL) __fnPtr[1](__arg[1]);
}

__VOID	__intVect_02(__VOID) {

	if (__fnPtr[2] != __NULL) __fnPtr[2](__arg[2]);
}

__VOID	__intVect_03(__VOID) {

	if (__fnPtr[3] != __NULL) __fnPtr[3](__arg[3]);
}

__VOID	__intVect_04(__VOID) {

	if (__fnPtr[4] != __NULL) __fnPtr[4](__arg[4]);
}

__VOID	__intVect_05(__VOID) {

	if (__fnPtr[5] != __NULL) __fnPtr[5](__arg[5]);
}

__VOID	__intVect_06(__VOID) {

	if (__fnPtr[6] != __NULL) __fnPtr[6](__arg[6]);
}

__VOID	__intVect_07(__VOID) {

	if (__fnPtr[7] != __NULL) __fnPtr[7](__arg[7]);
}

__VOID	__intVect_08(__VOID) {

	if (__fnPtr[8] != __NULL) __fnPtr[8](__arg[8]);
}

__VOID	__intVect_09(__VOID) {

	if (__fnPtr[9] != __NULL) __fnPtr[9](__arg[9]);
}

__VOID	__intVect_10(__VOID) {

	if (__fnPtr[10] != __NULL) __fnPtr[10](__arg[10]);
}

__VOID	__intVect_11(__VOID) {

	if (__fnPtr[11] != __NULL) __fnPtr[11](__arg[11]);
}

__VOID	__intVect_12(__VOID) {

	if (__fnPtr[12] != __NULL) __fnPtr[12](__arg[12]);
}

__VOID	__intVect_13(__VOID) {

	if (__fnPtr[13] != __NULL) __fnPtr[13](__arg[13]);
}

__VOID	__intVect_14(__VOID) {

	if (__fnPtr[14] != __NULL) __fnPtr[14](__arg[14]);
}

__VOID	__intVect_15(__VOID) {

	if (__fnPtr[15] != __NULL) __fnPtr[15](__arg[15]);
}

__VOID	__intVect_16(__VOID) {

	if (__fnPtr[16] != __NULL) __fnPtr[16](__arg[16]);
}

__VOID	__intVect_17(__VOID) {

	if (__fnPtr[17] != __NULL) __fnPtr[17](__arg[17]);
}

__VOID	__intVect_18(__VOID) {

	if (__fnPtr[18] != __NULL) __fnPtr[18](__arg[18]);
}

__VOID	__intVect_19(__VOID) {

	if (__fnPtr[19] != __NULL) __fnPtr[19](__arg[19]);
}

__VOID	__intVect_20(__VOID) {

	if (__fnPtr[20] != __NULL) __fnPtr[20](__arg[20]);
}

__VOID	__intVect_21(__VOID) {

	if (__fnPtr[21] != __NULL) __fnPtr[21](__arg[21]);
}

__VOID	__intVect_22(__VOID) {

	if (__fnPtr[22] != __NULL) __fnPtr[22](__arg[22]);
}

__VOID	__intVect_23(__VOID) {

	if (__fnPtr[23] != __NULL) __fnPtr[23](__arg[23]);
}

__VOID	__intVect_24(__VOID) {

	if (__fnPtr[24] != __NULL) __fnPtr[24](__arg[24]);
}

__VOID	__intVect_25(__VOID) {

	if (__fnPtr[25] != __NULL) __fnPtr[25](__arg[25]);
}

__VOID	__intVect_26(__VOID) {

	if (__fnPtr[26] != __NULL) __fnPtr[26](__arg[26]);
}

__VOID	__intVect_27(__VOID) {

	if (__fnPtr[27] != __NULL) __fnPtr[27](__arg[27]);
}

__VOID	__intVect_28(__VOID) {

	if (__fnPtr[28] != __NULL) __fnPtr[28](__arg[28]);
}

__VOID	__intVect_29(__VOID) {

	if (__fnPtr[29] != __NULL) __fnPtr[29](__arg[29]);
}

__VOID	__intVect_30(__VOID) {

	if (__fnPtr[30] != __NULL) __fnPtr[30](__arg[30]);
}

__VOID	__intVect_31(__VOID) {

	if (__fnPtr[31] != __NULL) __fnPtr[31](__arg[31]);
}

__VOID	__intVect_32(__VOID) {

	if (__fnPtr[32] != __NULL) __fnPtr[32](__arg[32]);
}

__VOID	__intVect_33(__VOID) {

	if (__fnPtr[33] != __NULL) __fnPtr[33](__arg[33]);
}

__VOID	__intVect_34(__VOID) {

	if (__fnPtr[34] != __NULL) __fnPtr[34](__arg[34]);
}

__VOID	__intVect_35(__VOID) {

	if (__fnPtr[35] != __NULL) __fnPtr[35](__arg[35]);
}

__VOID	__intVect_36(__VOID) {

	if (__fnPtr[36] != __NULL) __fnPtr[36](__arg[36]);
}

__VOID	__intVect_37(__VOID) {

	if (__fnPtr[37] != __NULL) __fnPtr[37](__arg[37]);
}

__VOID	__intVect_38(__VOID) {

	if (__fnPtr[38] != __NULL) __fnPtr[38](__arg[38]);
}

__VOID	__intVect_39(__VOID) {

	if (__fnPtr[39] != __NULL) __fnPtr[39](__arg[39]);
}

__VOID	__intVect_40(__VOID) {

	if (__fnPtr[40] != __NULL) __fnPtr[40](__arg[40]);
}

__VOID	__intVect_41(__VOID) {

	if (__fnPtr[41] != __NULL) __fnPtr[41](__arg[41]);
}

__VOID	__intVect_42(__VOID) {

	if (__fnPtr[42] != __NULL) __fnPtr[42](__arg[42]);
}

__VOID	__intVect_43(__VOID) {

	if (__fnPtr[43] != __NULL) __fnPtr[43](__arg[43]);
}

#endif /* __PLATCONFIG_GET_IRQ_SOURCE */

/*!
 * @brief Set interrupt vector.
 *
 * Call this function to add a function pointer into the __fnPtr
 * vector table.
 *
 * @param vecnum	Number of vector (zero-based).
 * @param func		Pointer to a @ref __INTFUNC type function.
 * @param arg		Void pointer to a custom parameter (to pass each time
 * 					the \c func function is called.
 *
 * @return			Nothing.
 *
 */
__VOID	__intSetVector(u8 vecnum, __INTFUNC *func, __PVOID arg) {

	if (vecnum < __PLATCONFIG_MAX_IRQ) {
		__fnPtr[vecnum] = func;
		__arg[vecnum] = arg;
	}
}
