/***************************************************************************
 * thlist.h
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

#ifndef __THLIST_H__
#define __THLIST_H__

#include <plat_ostypes.h>
#include <core/inc/thread.h>

void __thlAddPrioList(__PTHREAD th, __PTHREAD* thb);
void __thlRemovePrioList(__PTHREAD* thb);
void __thlAddSuspList(__PTHREAD th, __PTHREAD* thl);
void __thlRemoveSuspList(__PTHREAD th, __PTHREAD* thb, __PTHREAD* thl);
void __thlAddEvtPrio(__PTHREAD th, __PTHREAD* the);
void __thlRemoveEvtPrio(__PTHREAD th, __PTHREAD* the);

#endif /* __THLIST_H__ */
