/**
 * @file echo.h
 * @brief Echo protocol
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

#ifndef _ECHO_H
#define _ECHO_H

//Dependencies
#include "os.h"
#include "socket.h"

//Stack size required to run the echo service
#define ECHO_SERVICE_STACK_SIZE 600
//Priority at which the echo service should run
#define ECHO_SERVICE_PRIORITY 2
//Size of the echo buffer
#define ECHO_BUFFER_SIZE 1500
//Maximum time the TCP echo server will wait before closing the connection
#define ECHO_TIMEOUT 20000

//Echo service port
#define ECHO_PORT 7


/**
 * @brief Echo service context
 **/

typedef struct
{
   Socket *socket;
   char_t buffer[ECHO_BUFFER_SIZE];
} EchoServiceContext;


//TCP echo service related functions
error_t tcpEchoStart(void);
void tcpEchoListenerTask(void *param);
void tcpEchoConnectionTask(void *param);

//UDP echo service related functions
error_t udpEchoStart(void);
void udpEchoTask(void *param);

#endif
