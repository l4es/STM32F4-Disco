/**
 * @file main.c
 * @brief Main routine
 *
 * @section License
 *
 * Copyright (C) 2010-2013 Oryx Embedded. All rights reserved.
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

//Dependencies
#include <stdlib.h>
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "os.h"

#include "tcp_ip_stack.h"
#include "stm32f4x7_eth.h"
#include "dp83848.h"
#include "dhcp_client.h"
#include "smtp_client.h"
#include "yarrow.h"
#include "http_server.h"
#include "mime.h"
#include "str.h"
#include "resource_manager.h"
#include "debug.h"

//Constant definitions
#define USE_DHCP 			1
#define IPV4_STATIC_IP 		"192.168.2.80"
#define IPV4_STATIC_MASK	"255.255.255.0"
#define IPV4_STATIC_GW		"192.168.2.1"
#define IPV4_STATIC_DNS0	"192.168.2.1"
#define IPV4_STATIC_DNS1	"212.27.40.240"
#define CLIENT_SERVER_PORT 	80
#define CLIENT_SERVER_NAME 	"www.mikrocontroller.net"
#define CLIENT_REQUEST_URI 	"/"

//Forward declaration of functions
error_t httpServerCgiCallback(HttpConnection *connection, const char_t *param);
error_t httpServerUriNotFoundCallback(HttpConnection *connection);

//Global variables
YarrowContext yarrowContext;
uint8_t seed[32];

/**
 * @brief HTTP client test routine
 * @return Error code
 **/

error_t httpClientTest(void)
{
   error_t error;
   size_t length;
   IpAddr ipAddr;
   Socket *socket;
   static char_t buffer[256];

   //Debug message
   TRACE_INFO("\r\n\r\nResolving server name...\r\n");

   //Resolve HTTP server name
   error = getHostByName(NULL, CLIENT_SERVER_NAME, &ipAddr, 1, NULL, 0);
   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_INFO("Failed to resolve server name!\r\n");
      //Exit immedialtely
      return error;
   }

   //Create a new socket to handle the request
   socket = socketOpen(SOCKET_TYPE_STREAM, SOCKET_PROTOCOL_TCP);
   //Any error to report?
   if(!socket)
   {
      //Debug message
      TRACE_INFO("Failed to open socket!\r\n");
      //Exit immedialtely
      return error;
   }

   //Start of exception handling block
   do
   {
      //Debug message
      TRACE_INFO("Connecting to HTTP server %s\r\n", ipAddrToString(&ipAddr, NULL));

      //Connect to the HTTP server
      error = socketConnect(socket, &ipAddr, CLIENT_SERVER_PORT);
      //Any error to report?
      if(error) break;

      //Debug message
      TRACE_INFO("Successful connection\r\n");

      //Format HTTP request
      length = sprintf(buffer, "GET %s HTTP/1.0\r\nHost: %s:%u\r\n\r\n",
         CLIENT_REQUEST_URI, CLIENT_SERVER_NAME, CLIENT_SERVER_PORT);

      //Debug message
      TRACE_INFO("\r\nHTTP request:\r\n%s", buffer);

      //Send HTTP request
      error = socketSend(socket, buffer, length, NULL, 0);
      //Any error to report?
      if(error) break;

      //Debug message
      TRACE_INFO("HTTP response header:\r\n");

      //Parse HTTP response header
      while(1)
      {
         //Read the header line by line
         error = socketReceive(socket, buffer, sizeof(buffer) - 1, &length, SOCKET_FLAG_BREAK_CRLF);
         //End of stream?
         if(error) break;

         //Properly terminate the string with a NULL character
         buffer[length] = '\0';
         //Dump current data
         TRACE_INFO("%s", buffer);

         //The end of the header has been reached?
         if(!strcmp(buffer, "\r\n"))
            break;
      }

      //Debug message
      TRACE_INFO("HTTP response body:\r\n");

      //Parse HTTP response body
      while(1)
      {
         //Read response body
         error = socketReceive(socket, buffer, sizeof(buffer) - 1, &length, 0);
         //End of stream?
         if(error) break;

         //Properly terminate the string with a NULL character
         buffer[length] = '\0';
         //Dump current data
         TRACE_INFO("%s", buffer);
      }

      //End of exception handling block
   } while(0);

   //Close the connection
   socketClose(socket);
   //Debug message
   TRACE_INFO("\r\nConnection closed...\r\n");

   //Return status code
   return error;
}


/**
 * @brief User task
 **/

void userTask(void *param)
{
	NetInterface* nif = param;

   //Endless loop
   while(1)
   {
      // was DHCP succesfull and is link up?
	  if(nif->ipv4Config.addr != 0 && nif->linkState)
      {
    	  STM_EVAL_LEDOn(LED4);
      }
      else
      {
		  STM_EVAL_LEDOff(LED4);
      }

      //User button pressed?
      if(STM_EVAL_PBGetState(BUTTON_USER))
      {
    	 STM_EVAL_LEDOn(LED3);

    	 //HTTP client test routine
         httpClientTest();

         STM_EVAL_LEDOff(LED3);

         //Wait for the user button to be released
         while(STM_EVAL_PBGetState(BUTTON_USER));
      }

      //100ms delay
      osDelay(100);
   }
}


/**
 * @brief LED blinking task
 **/

void blinkTask(void *parameters)
{
   while(1)
   {
      STM_EVAL_LEDOn(LED5);
      osDelay(100);
      STM_EVAL_LEDOff(LED5);
      osDelay(900);
   }
}


/**
 * @brief Main entry point
 * @return Unused value
 **/

int_t main(void)
{
   error_t error;
   NetInterface *interface;
   OsTask *task;

#if USE_DHCP
   static DhcpClientSettings dhcpClientSettings;
   static DhcpClientCtx dhcpClientContext;
#endif
   static HttpServerSettings httpServerSettings;
   static HttpServerContext httpServerContext;

   SystemInit();

   //Configure debug UART
   debugInit();

   //Start-up message
   TRACE_INFO("\r\n");
   TRACE_INFO("***********************************\r\n");
   TRACE_INFO("*** CycloneTCP HTTP Client Demo ***\r\n");
   TRACE_INFO("***********************************\r\n");
   TRACE_INFO("Copyright: 2010-2013 Oryx Embedded\r\n");
   TRACE_INFO("Compiled: %s %s\r\n", __DATE__, __TIME__);
   TRACE_INFO("Target: STM32F407\r\n");
   TRACE_INFO("\r\n");

   //LED configuration
   STM_EVAL_LEDInit(LED3);
   STM_EVAL_LEDInit(LED4);
   STM_EVAL_LEDInit(LED5);
   STM_EVAL_LEDInit(LED6);

   //Clear LEDs
   STM_EVAL_LEDOff(LED3);
   STM_EVAL_LEDOff(LED4);
   STM_EVAL_LEDOff(LED5);
   STM_EVAL_LEDOff(LED6);

   //Initialize user button
   STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);

   //PRNG initialization
   error = yarrowInit(&yarrowContext);

   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize PRNG!\r\n");
   }

   //Generate a random seed

   //Properly seed the PRNG
   error = yarrowSeed(&yarrowContext, seed, sizeof(seed));

   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to seed PRNG!\r\n");
   }

   //TCP/IP stack initialization
   error = tcpIpStackInit();

   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize TCP/IP stack!\r\n");
   }

   //Configure the first Ethernet interface
   interface = &netInterface[0];
   //Select the relevant network adapter
   interface->nicDriver = &stm32f4x7EthDriver;
   interface->phyDriver = &dp83848PhyDriver;
   //Interface name
   strcpy(interface->name, "eth0");
   //Set host MAC address
   macStringToAddr("00-AB-CD-EF-04-07", &interface->macAddr);

#if (IPV6_SUPPORT == ENABLED)
   //Set link-local IPv6 address
   ipv6StringToAddr("fe80::00ab:cdef:0407", &interface->ipv6Config.linkLocalAddr);
#endif

   //Initialize network interface
   error = tcpIpStackConfigInterface(interface);

   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to configure interface %s!\r\n", interface->name);
   }

#if USE_DHCP
   //Set the network interface to be configured by DHCP
   dhcpClientSettings.interface = &netInterface[0];
   //Disable rapid commit option
   dhcpClientSettings.rapidCommit = FALSE;
   //Start DHCP client
   error = dhcpClientStart(&dhcpClientContext, &dhcpClientSettings);

   //Failed to start DHCP client?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to start DHCP client!\r\n");
   }
#else
   //Manual configuration
   interface = &netInterface[0];

   //IPv4 address
   ipv4StringToAddr(IPV4_STATIC_IP, &interface->ipv4Config.addr);
   //Subnet mask
   ipv4StringToAddr(IPV4_STATIC_MASK, &interface->ipv4Config.subnetMask);
   //Default gateway
   ipv4StringToAddr(IPV4_STATIC_GW, &interface->ipv4Config.defaultGateway);

   //Primary and secondary DNS servers
   interface->ipv4Config.dnsServerCount = 2;
   ipv4StringToAddr(IPV4_STATIC_DNS0, &interface->ipv4Config.dnsServer[0]);
   ipv4StringToAddr(IPV4_STATIC_DNS1, &interface->ipv4Config.dnsServer[1]);
#endif
   //Bind HTTP server to the desired interface
   httpServerSettings.interface = &netInterface[0];
   //Listen to port 80
   httpServerSettings.port = HTTP_PORT;
   //Specify the server's root directory
   strcpy(httpServerSettings.rootDirectory, "/www/");
   //Set default home page
   strcpy(httpServerSettings.defaultDocument, "index.shtm");
   //Callback functions
   httpServerSettings.cgiCallback = httpServerCgiCallback;
   httpServerSettings.uriNotFoundCallback = httpServerUriNotFoundCallback;
   //Start HTTP server
   error = httpServerStart(&httpServerContext, &httpServerSettings);

   //Failed to start HTTP server?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to start HTTP server!\r\n");
   }

   //Create user task
   task = osTaskCreate("User Task", userTask, interface, 500, 1);
   //Failed to create the task?
   if(task == OS_INVALID_HANDLE)
   {
      //Debug message
      TRACE_ERROR("Failed to create task!\r\n");
   }

   //Create a task to blink the LED
   task = osTaskCreate("Blink", blinkTask, NULL, 500, 1);
   //Failed to create the task?
   if(task == OS_INVALID_HANDLE)
   {
      //Debug message
      TRACE_ERROR("Failed to create task!\r\n");
   }

   //Start the execution of tasks
   osStart();

   //This function should never return
   return 0;
}

error_t httpServerCgiCallback(HttpConnection *connection, const char_t *param)
{
   static uint_t pageCounter = 0;
   uint_t length;

   //Underlying network interface
   NetInterface *interface = connection->socket->interface;

   //Check parameter name
   if(!strcasecmp(param, "PAGE_COUNTER"))
   {
      pageCounter++;
      sprintf(connection->buffer, "%u time%s", pageCounter, (pageCounter >= 2) ? "s" : "");
   }
   else if(!strcasecmp(param, "BOARD_NAME"))
   {
      strcpy(connection->buffer, "STM3240G-EVAL");
   }
   else if(!strcasecmp(param, "SYSTEM_TIME"))
   {
      time_t time = osGetTickCount();
      sprintf(connection->buffer, "%lus %03lums", time / 1000, time % 1000);
   }
   else if(!strcasecmp(param, "MAC_ADDR"))
   {
      macAddrToString(&interface->macAddr, connection->buffer);
   }
   else if(!strcasecmp(param, "IPV4_ADDR"))
   {
      ipv4AddrToString(interface->ipv4Config.addr, connection->buffer);
   }
   else if(!strcasecmp(param, "SUBNET_MASK"))
   {
      ipv4AddrToString(interface->ipv4Config.subnetMask, connection->buffer);
   }
   else if(!strcasecmp(param, "DEFAULT_GATEWAY"))
   {
      ipv4AddrToString(interface->ipv4Config.defaultGateway, connection->buffer);
   }
   else if(!strcasecmp(param, "IPV4_PRIMARY_DNS"))
   {
      ipv4AddrToString(interface->ipv4Config.dnsServer[0], connection->buffer);
   }
   else if(!strcasecmp(param, "IPV4_SECONDARY_DNS"))
   {
      ipv4AddrToString(interface->ipv4Config.dnsServer[1], connection->buffer);
   }
#if (IPV6_SUPPORT == ENABLED)
   else if(!strcasecmp(param, "LINK_LOCAL_ADDR"))
   {
      ipv6AddrToString(&interface->ipv6Config.linkLocalAddr, connection->buffer);
   }
   else if(!strcasecmp(param, "GLOBAL_ADDR"))
   {
      ipv6AddrToString(&interface->ipv6Config.globalAddr, connection->buffer);
   }
   else if(!strcasecmp(param, "IPV6_PREFIX"))
   {
      ipv6AddrToString(&interface->ipv6Config.prefix, connection->buffer);
      length = strlen(connection->buffer);
      sprintf(connection->buffer + length, "/%u", interface->ipv6Config.prefixLength);
   }
   else if(!strcasecmp(param, "ROUTER"))
   {
      ipv6AddrToString(&interface->ipv6Config.router, connection->buffer);
   }
   else if(!strcasecmp(param, "IPV6_PRIMARY_DNS"))
   {
      ipv6AddrToString(&interface->ipv6Config.dnsServer[0], connection->buffer);
   }
   else if(!strcasecmp(param, "IPV6_SECONDARY_DNS"))
   {
      ipv6AddrToString(&interface->ipv6Config.dnsServer[1], connection->buffer);
   }
#endif
   else
   {
      return ERROR_INVALID_TAG;
   }

   //Get the length of the resulting string
   length = strlen(connection->buffer);

   //Send the contents of the specified environment variable
   return httpWriteStream(connection, connection->buffer, length);
}

error_t httpServerUriNotFoundCallback(HttpConnection *connection)
{
   error_t error;
   uint_t i;
   uint_t j;
   uint_t n;
   char_t *buffer;

   //Process data.xml file?
   if(!strcasecmp(connection->request.uri, "/data.xml"))
   {
      //Point to the scratch buffer
      buffer = connection->buffer + 384;

      //Format XML data
      n = sprintf(buffer, "<data>\r\n");
      n += sprintf(buffer + n, "  <ax>%d</ax>\r\n", 0); // fixme
      n += sprintf(buffer + n, "  <ay>%d</ay>\r\n", 0); // fixme
      n += sprintf(buffer + n, "  <az>%d</az>\r\n", 0); // fixme
      n += sprintf(buffer + n, "  <adc>%u</adc>\r\n", 100); // fixme
      n += sprintf(buffer + n, "  <joystick>%u</joystick>\r\n", 0); // fixme

      //End of XML data
      n += sprintf(buffer + n, "</data>\r\n");

      //Format HTTP response header
      connection->response.version = connection->request.version;
      connection->response.statusCode = 200;
      connection->response.keepAlive = connection->request.keepAlive;
      connection->response.noCache = TRUE;
      connection->response.contentType = mimeGetType(".xml");
      connection->response.chunkedEncoding = FALSE;
      connection->response.contentLength = n;

      //Send the header to the client
      error = httpWriteHeader(connection);
      //Any error to report?
      if(error) return error;

      //Send response body
      error = httpWriteStream(connection, buffer, n);
      //Any error to report?
      if(error) return error;

      //Properly close output stream
      error = httpCloseStream(connection);
      //Return status code
      return error;
   }
   //Process send_mail.xml file?
   else if(!strcasecmp(connection->request.uri, "/send_mail.xml"))
   {
      char *separator;
      char *property;
      char *value;
      char *p;
      SmtpAuthInfo authInfo;
      SmtpMail mail;
      SmtpMailAddr recipients[4];

      //Initialize structures to zero
      memset(&authInfo, 0, sizeof(authInfo));
      memset(&mail, 0, sizeof(mail));
      memset(recipients, 0, sizeof(recipients));

      //Set the relevant PRNG algorithm to be used
      authInfo.prngAlgo = YARROW_PRNG_ALGO;
      authInfo.prngContext = &yarrowContext;

      //Set email recipients
      mail.recipients = recipients;
      //Point to the scratch buffer
      buffer = connection->buffer;

      //Start of exception handling block
      do
      {
         //Process HTTP request body
         while(1)
         {
            //Read the HTTP request body until a ampersand is encountered
            error = httpReadStream(connection, buffer,
               HTTP_SERVER_BUFFER_SIZE - 1, &n, HTTP_FLAG_BREAK('&'));
            //End of stream detected?
            if(error) break;

            //Properly terminate the string with a NULL character
            buffer[n] = '\0';

            //Remove the trailing ampersand
            if(n > 0 && buffer[n - 1] == '&')
               buffer[--n] = '\0';

            //Decode the percent-encoded string
            for(i = 0, j = 0; i < n; i++, j++)
            {
               //Replace '+' characters with spaces
               if(buffer[i] == '+')
               {
                  buffer[j] = ' ';
               }
               //Process percent-encoded characters
               else if(buffer[i] == '%' && (i + 2) < n)
               {
                  buffer[i] = buffer[i + 1];
                  buffer[i + 1] = buffer[i + 2];
                  buffer[i + 2] = '\0';
                  buffer[j] = strtoul(buffer + i, NULL, 16);
                  i += 2;
               }
               //Copy any other characters
               else
               {
                  buffer[j] = buffer[i];
               }
            }

            //Properly terminate the resulting string
            buffer[j] = '\0';

            //Check whether a separator is present
            separator = strchr(buffer, '=');

            //Separator found?
            if(separator)
            {
               //Split the line
               *separator = '\0';
               //Get property name and value
               property = strTrimWhitespace(buffer);
               value = strTrimWhitespace(separator + 1);

               //Check property name
               if(!strcasecmp(property, "server"))
               {
                  //Save server name
                  authInfo.serverName = strDuplicate(value);
               }
               else if(!strcasecmp(property, "port"))
               {
                  //Save the server port to be used
                  authInfo.serverPort = atoi(value);
               }
               else if(!strcasecmp(property, "userName"))
               {
                  //Save user name
                  authInfo.userName = strDuplicate(value);
               }
               else if(!strcasecmp(property, "password"))
               {
                  //Save password
                  authInfo.password = strDuplicate(value);
               }
               else if(!strcasecmp(property, "useTls"))
               {
                  //Open a secure SSL/TLS session?
                  authInfo.useTls = TRUE;
               }
               else if(!strcasecmp(property, "recipient"))
               {
                  //Split the recipient address list
                  value = strtok_r(value, ", ", &p);

                  //Loop through the list
                  while(value != NULL)
                  {
                     //Save recipient address
                     recipients[mail.recipientCount].name = NULL;
                     recipients[mail.recipientCount].addr = strDuplicate(value);
                     recipients[mail.recipientCount].type = SMTP_RCPT_TYPE_TO;
                     //Get the next item in the list
                     value = strtok_r(NULL, ", ", &p);

                     //Increment the number of recipients
                     if(++mail.recipientCount >= arraysize(recipients))
                        break;
                  }
               }
               else if(!strcasecmp(property, "from"))
               {
                  //Save sender address
                  mail.from.name = NULL;
                  mail.from.addr = strDuplicate(value);
               }
               else if(!strcasecmp(property, "date"))
               {
                  //Save current time
                  mail.dateTime = strDuplicate(value);
               }
               else if(!strcasecmp(property, "subject"))
               {
                  //Save mail subject
                  mail.subject = strDuplicate(value);
               }
               else if(!strcasecmp(property, "body"))
               {
                  //Save mail body
                  mail.body = strDuplicate(value);
               }
            }
         }

         //Propagate exception if necessary
         if(error != ERROR_END_OF_STREAM)
            break;

         //Send mail
         error = smtpSendMail(&authInfo, &mail);

         //Point to the scratch buffer
         buffer = connection->buffer + 384;
         //Format XML data
         n = sprintf(buffer, "<data>\r\n  <status>");

         if(error == NO_ERROR)
            n += sprintf(buffer + n, "Mail successfully sent!\r\n");
         else if(error == ERROR_NAME_RESOLUTION_FAILED)
            n += sprintf(buffer + n, "Cannot resolve SMTP server name!\r\n");
         else if(error == ERROR_AUTHENTICATION_FAILED)
            n += sprintf(buffer + n, "Authentication failed!\r\n");
         else if(error == ERROR_UNEXPECTED_RESPONSE)
            n += sprintf(buffer + n, "Unexpected response from SMTP server!\r\n");
         else
            n += sprintf(buffer + n, "Failed to send mail (error %d)!\r\n", error);

         n += sprintf(buffer + n, "</status>\r\n</data>\r\n");

         //Format HTTP response header
         connection->response.version = connection->request.version;
         connection->response.statusCode = 200;
         connection->response.keepAlive = connection->request.keepAlive;
         connection->response.noCache = TRUE;
         connection->response.contentType = mimeGetType(".xml");
         connection->response.chunkedEncoding = FALSE;
         connection->response.contentLength = n;

         //Send the header to the client
         error = httpWriteHeader(connection);
         //Any error to report?
         if(error) break;

         //Send response body
         error = httpWriteStream(connection, buffer, n);
         //Any error to report?
         if(error) break;

         //Properly close output stream
         error = httpCloseStream(connection);
         //Any error to report?
         if(error) break;

         //End of exception handling block
      } while(0);

      //Free previously allocated memory
      osMemFree((void *) authInfo.serverName);
      osMemFree((void *) authInfo.userName);
      osMemFree((void *) authInfo.password);
      osMemFree((void *) recipients[0].addr);
      osMemFree((void *) mail.from.addr);
      osMemFree((void *) mail.dateTime);
      osMemFree((void *) mail.subject);
      osMemFree((void *) mail.body);

      //Return status code
      return error;
   }
   else
   {
      return ERROR_NOT_FOUND;
   }
}

