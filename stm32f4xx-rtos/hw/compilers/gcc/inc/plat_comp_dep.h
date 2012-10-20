/***************************************************************************
 * plat_comp_dep.h
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
#ifndef __PLAT_COMP_DEP_H_
#define __PLAT_COMP_DEP_H_

#include <plat_ostypes.h>

/** @addtogroup Platform_STM32_Compiler
  * @{
  */

/** @defgroup Platform_STM32_Compiler_GCC GCC
  * @{
  */

#define __pcd_EnableIRQs() 	{__asm __VOLATILE (	"CPSIE	I\n");	}
#define __pcd_DisableIRQs()	{__asm __VOLATILE (	"CPSID	I\n");	}

__VOID __pcd_ZeroPSP(__VOID);
__VOID __pcd_SetSvcPend(__VOID);
__VOID __pcd_PendSVHandler(__VOID) __attribute__ ((naked));

extern unsigned long _heap_start;		/*!< @brief Start of heap, from linker file */
extern unsigned long _heap_avail;		/*!< @brief Available heap, from linker file */

#define __CPU_HEAP_BASE	_heap_start		/*!< @brief Define here the start of heap */
#define __CPU_HEAP_SIZE	_heap_avail		/*!< @brief Define here the size of heap */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __PLAT_COMP_DEP_H_ */
