/***************************************************************************
 * plat_comp_dep.c
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

#include <plat_ostypes.h>

/** @addtogroup Platform_STM32_Compiler_GCC
  * @{
  */

/*!
 * @brief PenSV interrupt handler
 *
 * Main context-switch function. ISR for Cortex PendSV interrupt
 *
*/
void __attribute__((naked)) __pcd_PendSVHandler(void) {
/* See AN298 - Cortex-M4(F) Lazy Stacking and Context Switching (DAI0298A_cortex_m4f_lazy_stacking_and_context_switching.pdf) */
	__asm __VOLATILE (

	"CPSID	I\n"
    "MRS	R0, PSP\n"				// Get the process stack pointer
    "CBZ	R0, OSPendSVSKIP\n"		// Skip saving register on first pass

    "SUBS	R0, R0, 0x20\n"			// Set position 0x20 bytes on top
    "STM	R0, {R4-R11}\n"			// and save the register the CORTEX did not save (R4-R11)

	"LDR	R1, =__threadSp\n"		// Update current thread SP variable
	"LDR	R1, [R1]\n"
	"STR	R0, [R1]\n"				// used to save last on context switch

	// ===== CONTEXT SAVED ======

	"OSPendSVSKIP:\n"
    "PUSH	{R14}\n"				// need to save LR exc_return value
    "LDR	R0, =__threadChange\n"
    "BLX	R0\n"
    "POP	{R14}\n"

    "LDR	R0, =__threadSp\n"		// R0 is new PSP
	"LDR	R0, [R0]\n"
	"LDR	R0, [R0]\n"
    "LDM	R0, {R4-R11}\n"			// get r4-11
    "ADDS	R0, R0, 0x20\n"

    "MSR	PSP, R0\n"				// load PSP with new process SP
    "ORR	LR, LR, 0x04\n"			// when irq returns should find process stack
	"CPSIE  I\n"
	"BX 	LR\n"
	);
}

/*!
 * @brief Set PSP to zero
 *
*/
__VOID __pcd_ZeroPSP(__VOID) {
	__asm __VOLATILE (	"PUSH	{R0}\n"
						"MOVS	R0, #0\n"	// Set the PSP to 0 for initial context switch call
						"MSR	PSP, R0\n"
						"POP	{R0}\n");
}

/*!
 * @brief Queue PendSV interrupt
 *
*/

__VOID __pcd_SetSvcPend(__VOID) {

	__asm __VOLATILE (	"LDR	R0, =0xE000ED22\n"			// System priority register (2) - See Cortex-M4 Devices Generic User Guide (DUI0553A_cortex_m4_dgug.pdf) - page 237
						"LDR	R1, =0xFF\n"				// PendSV priority value (highest)
						"STRB	R1, [R0]\n"
						"NOP\n");
}

/**
  * @}
  */

