/**
 * @file chargen.h
 * @brief Character generator protocol
 *
 * @section License
 *
 * Copyright (C) 2010-2013 Oryx Embedded. All rights reserved.
 *
 * This file is part of CycloneTCP Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded (www.oryx-embedded.com)
 * @version 1.3.5
 **/

#ifndef _CHARGEN_H
#define _CHARGEN_H

//Dependencies
#include "os.h"
#include "socket.h"

//Stack size required to run the chargen service
#define CHARGEN_SERVICE_STACK_SIZE 600
//Priority at which the chargen service should run
#define CHARGEN_SERVICE_PRIORITY 2
//Size of the chargen buffer
#define CHARGEN_BUFFER_SIZE 1500
//Maximum time the TCP chargen server will wait before closing the connection
#define CHARGEN_TIMEOUT 20000

//Chargen service port
#define CHARGEN_PORT 19


/**
 * @brief Chargen service context
 **/

typedef struct
{
   Socket *socket;
   char_t buffer[CHARGEN_BUFFER_SIZE];
} ChargenServiceContext;


//TCP chargen service related functions
error_t tcpChargenStart(void);
void tcpChargenListenerTask(void *param);
void tcpChargenConnectionTask(void *param);

//UDP chargen service related functions
error_t udpChargenStart(void);
void udpChargenTask(void *param);

#endif
