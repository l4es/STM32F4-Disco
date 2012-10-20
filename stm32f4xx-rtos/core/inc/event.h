/***************************************************************************
 * event.h
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

#ifndef	__EVENT_H__
#define	__EVENT_H__

#include <plat_ostypes.h>
#include "thread.h"

/** @addtogroup Event
  * @{
  */

/** @defgroup Event_Constants Constants
  * @{
  */

#define __EV_INFINITE	0x00

/** @defgroup Event_ReturnValuesDefines Return values
  * @brief Return values for __eventWait() function.
  * @{
  */

#define	__EVRET_TIMEOUT		0x00	/*!< @brief Event timeout */
#define	__EVRET_SUCCESS		0x01	/*!< @brief Event was arrived */
#define __EVRET_LIST_FULL	0x02	/*!< @brief Waiting list full */
#define __EVRET_ABORT		0x03	/*!< @brief Event aborted */
#define	__EVRET_ERROR		0xFA	/*!< @brief Event error */

/**
  * @}
  */


/** @defgroup Event_StatesDefines Event states
  * @brief Possible event states.
  * @{
  */

#define __EV_RESET			0xFF	/*!< @brief Event ready to be tested*/
#define	__EV_SET			0xFE	/*!< @brief Event is served */
#define	__EV_SET_ERROR		0xFD	/*!< @brief Event is served but with errors */
#define	__EV_TIMEOUT		0xFC	/*!< @brief Event timeout */
#define	__EV_ABORT			0xFB	/*!< @brief Event aborted */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup Event_TypeDefs Typedefs
  * @{
  */

typedef struct __eventTag {

	u8				ev_state;		/*!< @brief Event state */
	__PTHREAD		ev_threads;		/*!< @brief list of waiting threads */
} __EVENT, *__PEVENT;

/**
  * @}
  */

__VOID __eventReset(__PEVENT ev);
u8 __eventWait(__PEVENT ev, u32 timeout);
__VOID __eventSet(__PEVENT ev);
__VOID __eventSetOne(__PEVENT ev);
__VOID __eventAbort(__PEVENT ev);
__PTHREAD __eventGetWaitingThreads(__PEVENT ev);

/**
  * @}
  */

#endif	/* __EVENT_H__ */
