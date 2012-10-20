/***************************************************************************
 * dbgterm.c
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

#include "dbgterm.h"
#include "heap.h"
#include "lock.h"
#include "device.h"
#include <common/inc/common.h>
#if __CONFIG_COMPILE_FAT
#include <fs/fat.h>
#endif

#if __CONFIG_COMPILE_NET
#include <net/icmp.h>
#include <net/arp.h>
#endif

#include __CONFIG_DBGTERM_IN_DEVICE_INC
#include __CONFIG_DBGTERM_OUT_DEVICE_INC

#if __CONFIG_DBGTERM_ENABLED

extern __BOOL __systemReset;

/** @addtogroup Core
  * @{
  */

/** @defgroup DbgTerminal Debug terminal
  * Manages debug commands for the @ref Terminal module.
  * @{
  */

/** @defgroup DbgTerminal_PrivateVariables Private variables
  * @{
  */

char* __thstatus[] = {	"READY", "RUNNING", "SLEEPING", "WAITING", "DISABLED",
						"FIRSTTIME", "DELETE", "WFL"};		/*!< @brief Strings for thread status */
__STATIC __PTERMINAL __dbgTerminal = __NULL;	/*!< @brief Debug Terminal structure */


/**
  * @}
  */

/** @defgroup DbgTerminal_PrivateTypedefs Private typedefs
  * @{
  */

typedef struct __heapWalkTag {
	u32 free;
	u32 busy;
	__PTERMINAL term;
} __HEAPWALK;

typedef struct __threadWalkTag {
	__PTHREAD th;
	u32 mem;
} __THREADWALK_MEM;


/**
  * @}
  */

/** @defgroup DbgTerminal_Functions Functions
  * @{
  */

/*!
 * @brief Callback for heap walk.
 *
 * Internal use. This function will be called for every memory block found in the heap.
 */
__STATIC __BOOL __dbgHeapCallBack(__PVOID ptr, u32 size, __BOOL fr, __PTHREAD th, __PVOID arg)
{
	__HEAPWALK* walk = (__HEAPWALK*) arg;

	if (fr == __TRUE)
	{
		walk->free += size;
		__terminalWriteLine(walk->term, "%08lXh %6lu free  ", ptr, size);
	} else {
		walk->busy += size;
		__terminalWriteLine(walk->term, "%08lXh %6lu busy   %s", ptr, size, (th) ? (__PSTRING) th->th_name : "not available");
	}

	return(__TRUE);
}

/*!
 * @brief Callback for thread walk.
 *
 * Internal use. This function will be called for thread found.
 */
__STATIC __BOOL __dbgThreadCallBack(__PVOID ptr, u32 size, __BOOL fr, __PTHREAD th, __PVOID arg)
{
	__THREADWALK_MEM* mem = (__THREADWALK_MEM*) arg;

	if (mem->th == th && fr == __FALSE) mem->mem += size;
	return(__TRUE);
}

/*!
 * @brief Entry point for "threads" terminal command.
 */
__STATIC __VOID	__dbgThreads(__PTERMINAL term)
{

	__PTHREAD	th = __threadGetChain();
	__THREADWALK_MEM mem;

	__terminalWriteLine(term, "\r\nCurrent threads:\r\n");
	__terminalWriteLine(term, "Name     Address   TTL    Stack Memory Prio State");
	__terminalWriteLine(term, "----------------------------------------------------");
	while (th)
	{
		mem.th = th;
		mem.mem = 0;

		__heapWalk(__dbgThreadCallBack, &mem);

		__terminalWriteLine(term, "%8s %08lXh %06lu %05lu %06lu %03lu %s",
						th->th_name,
						th,
						(u32) th->th_load,
						(u32) th->th_stksize,
						mem.mem,
						(u32) th->th_priority,
						__thstatus[th->th_status]);

		th = (__PTHREAD) th->th_lstnext;
	}
}

/*!
 * @brief Outputs lock information to the terminal.
 *
 * @return	Nothing.
 */
__VOID	__dbgLocks(__PTERMINAL term)
{
	__PLOCK_LIST list = __lockGetList();
	u32 count = __lockGetCount();

	__terminalWriteLine(term, "");

	if (!count)
	{
		__terminalWriteLine(term, "No locks defined");
		return;
	}

	__terminalWriteLine(term, "Locks defined: %lu", count);
	__terminalWriteLine(term, "");

	__terminalWriteLine(term, "Parent        Type     Status   Owner");
	__terminalWriteLine(term, "-------------------------------------");

	while(list)
	{
		if (list->lock->parent != __NULL)
		{
			__terminalWrite(term, "%14s", list->lock->parent->th_name);
		} else
		{
			__terminalWrite(term, "Not available ");
		}

		switch (list->lock->type)
		{
			case __LOCK_TYPE_NORMAL:	__terminalWrite(term, "%9s", "Normal");
										break;

			case __LOCK_TYPE_CPU:		__terminalWrite(term, "%9s", "CPU");
										break;
		}

		switch (list->lock->state)
		{
			case __LOCK_RELEASED: 	__terminalWrite(term, "%9s", "Released");
									break;

			case __LOCK_LOCKED:		__terminalWrite(term, "%9s", "Locked");
									break;
		}

		if (list->lock->owner != __NULL)
		{
			__terminalWrite(term, "%9s", list->lock->owner->th_name);
		} else
		{
			__terminalWrite(term, "%9s", "None");
		}

		__terminalWrite(term, "\r\n");

		list = list->next;
	}

	__terminalWriteLine(term, "");
}

/*!
 * @brief Outputs the devices list through the debug terminal.
 *
 * @return Nothing.
 */
__VOID __dbgDevices(__PTERMINAL term)
{
	__PDEVICE devs = __deviceGetList();

	__terminalWriteLine(term, "");

	if (!devs)
	{
		__terminalWriteLine(term, "There are no currently available devices.");
		return;
	}

	__terminalWriteLine(term, "Available devices:");
	__terminalWriteLine(term, "");

	__terminalWriteLine(term, "Name       Owner    Init   Open");
	__terminalWriteLine(term, "-------------------------------");

	while (devs)
	{
		__terminalWrite(term, "%8s   ", devs->dv_name);

		if (!devs->dv_owner)
		{
			__terminalWrite(term, "%9s", "None");
		} else {
			__terminalWrite(term, "%9s", devs->dv_owner);
		}

		__terminalWrite(term, "%4lu", (u32) devs->dv_initd);

		__terminalWriteLine(term, "%7lu", (u32) devs->dv_opcnt);

		devs = devs->dv_next;
	}

	__terminalWriteLine(term, "");
}

#if __CONFIG_COMPILE_NET

/*!
 * @brief Outputs the current ARP cache table to the debug terminal ("arp" command).
 *
 * @return Nothing.
 */
__VOID __dbgArpList(__PTERMINAL term)
{
	__PNETIF netif = __netifGetList();
	__PARP_ENTRY entry;
	__STRING ip[16];
	__STRING mac[18];

	while (netif)
	{
		__terminalWriteLine(term, "");
		__terminalWriteLine(term, "ARP table for interface %s\r\n", netif->dv->dv_name);
		__terminalWriteLine(term, "Address          HWAddress", netif->dv->dv_name);

		entry = netif->arp_table;
		if (entry)
		{
			while (entry)
			{
				__netIpToStr(entry->ip, ip);
				__netMacToStr(entry->mac, mac);
				__terminalWriteLine(term, "%16s %s", ip, mac);

				entry = entry->next;
			}

		} else {
			__terminalWriteLine(term, "No entries.");
		}

		__terminalWriteLine(term, "");
		netif = netif->next;
	}
}

#endif /* __CONFIG_COMPILE_NET */

#if __CONFIG_COMPILE_NET
__VOID __dbgNetif(__PTERMINAL term)
{
	__STRING str[32];
	__PNETIF netif = __netifGetList();

	while (netif)
	{
		__terminalWriteLine(term, "\r\nInformation for network interface %s\r\n", netif->dv->dv_name);

		__terminalWrite(term, "Type: ");

		switch (netif->type)
		{
			case __NETIF_PPP:
				__terminalWriteLine(term, "PPP");
				break;

			case __NETIF_ETH:
				__terminalWriteLine(term, "Ethernet");
				break;

			case __NETIF_LO:
				__terminalWriteLine(term, "Loopback");
				break;
		}

		if (netif->flags & __NETIF_FLAG_DHCP)
		{
			__terminalWriteLine(term, "Using DHCP");
		}

		__terminalWrite(term, "HWaddr: ");
		__netMacToStr(netif->addr.type.mac, str);
		__terminalWriteLine(term, str);

		__terminalWrite(term, "IP: ");
		__netIpToStr(netif->ip, str);
		__terminalWriteLine(term, str);

		__terminalWrite(term, "DNS1: ");
		__netIpToStr(netif->dns1, str);
		__terminalWriteLine(term, str);


		__terminalWrite(term, "DNS2: ");
		__netIpToStr(netif->dns2, str);
		__terminalWriteLine(term, str);

		__terminalWriteLine(term, "MTU: %lu", (u32) netif->mtu);

		netif = netif->next;
	}

	__terminalWriteLine(term, "");

}

#endif /* __CONFIG_COMPILE_NET */

/*!
 * @brief Debug terminal commands parser.
 *
 * This function will be called to check and process debug commands from the terminal.
 *
 * @param 	term 	The terminal from where the commands was written.
 * @param	str		Command string.
 * @param 	params	Command parameters.
 *
 * @return __TRUE if the command matches, otherwise __FALSE.
 */
__VOID __dbgTerminalIn(__PTERMINAL term, __PSTRING str, __PSTRING params)
{
	__HEAPWALK mem;
	u16 days;
	u8 hours;
	u8 minutes;
	u8 seconds;

	/* HEAP */
	if (__strCmp(str, "heap") == 0)
	{

		__terminalWriteLine(term, "\r\nThe current RAM heap:\r\n");
		__terminalWriteLine(term, "Address     Size State  Owner");
		__terminalWriteLine(term, "-------------------------------------");

		mem.free = mem.busy = 0;
		mem.term = term;
					
		__heapWalk(__dbgHeapCallBack, &mem);
		__terminalWriteLine(term, "");

		__terminalWriteLine(term, "Busy: %lu bytes", mem.busy);
		__terminalWriteLine(term, "Free: %lu bytes", mem.free);

		__terminalWriteLine(term, "");
		return;
	}
	
	/* THREADS */
	if (__strCmp(str, "threads") == 0)
	{
		__dbgThreads(term);
		__terminalWriteLine(term, (""));
		return;
	}
	
	/* RESET */
	if (__strCmp(str, "reset") == 0)
	{
		__systemReset = __TRUE;
		return;
	}
	
	/* LOCKS */
	if (__strCmp(str, "locks") == 0)
	{
		__dbgLocks(term);
		return;
	}
	
	/* LOCKS */
	if (__strCmp(str, "devices") == 0)
	{
		__dbgDevices(term);
		return;
	}

	/* DEFRAG */
	if (__strCmp(str, "defrag") == 0)
	{
		__systemDisableScheduler();
		__heapDefrag();
		__systemEnableScheduler();
		return;
	}

	/* UPTIME */
	if (__strCmp(str, "uptime") == 0)
	{
		days = __systemGetSecondsCount() / 86400;
		hours = (__systemGetSecondsCount() / 3600) - (days * 24);
		minutes = (__systemGetSecondsCount() / 60) - (days * 1440) - (hours * 60);
		seconds = __systemGetSecondsCount() % 60;

		__terminalWriteLine(term, "Uptime: %lu days, %lu hours, %lu minutes, %lu seconds",
						(u32) days,
						(u32) hours,
						(u32) minutes,
						(u32) seconds);

		__terminalWriteLine(term, (""));
		return;
	}

#if __CONFIG_COMPILE_FAT
	/* DIR */
	if (__strCmp(str, "dir") == 0)
	{
		__fatDirCmd(term);
		return;
	}

#endif

#if __CONFIG_COMPILE_NET
	/* PING */
	if (__strnCmpNoCase(str, "ping ", 5) == 0)
	{
		__icmpPing(str+5, 256, 4, 1000);
		return;
	}

	/* ARP */
	if (__strCmp(str, "arp") == 0)
	{
		__dbgArpList(term);
		return;
	}

	/* NETIFS */
	if (__strCmp(str, "netif") == 0)
	{
		__dbgNetif(term);
	}
#endif
}

__VOID __dbgWrite(__CONST __PSTRING str, ...)
{
	__PSTRING args = ((__PSTRING) &str) + sizeof(__PSTRING);
	__terminalOut(__dbgTerminal, __FALSE, str, args);
}

__VOID __dbgWriteLine(__CONST __PSTRING str, ...)
{
	__PSTRING args = ((__PSTRING) &str) + sizeof(__PSTRING);
	__terminalOut(__dbgTerminal, __TRUE, str, args);
}


__VOID __dbgInit(__VOID)
{
	__PDEVICE in, out;

	/* Find the debug terminal devices */
	in = __deviceFind(__CONFIG_DBGTERM_IN_DEVICE);
	out = __deviceFind(__CONFIG_DBGTERM_OUT_DEVICE);

	if (!in && !out)
		/* Devices not found */
		return;

	/* Start the input device */
	if (in)
	{
		if (__deviceInit(in, __NULL, __CONFIG_DBGTERM_IN_DEVICE_INIT_MODE) != __DEV_OK) return;

		if (__deviceOpen(in, __CONFIG_DBGTERM_IN_DEVICE_OPEN_MODE) != __DEV_OK)
		{
			__deviceDeinit(in);
			return;
		}

		__deviceIOCtl(in, __IOCTL_SETTXTIMEOUT, 20, __NULL, 0);
		__deviceIOCtl(in, __IOCTL_SETRXTIMEOUT, 20, __NULL, 0);
	}

	/* If the input device and the output device are not the same */
	if (out && in != out)
	{
		/* Init the output device */
		if (__deviceInit(out, __NULL, __CONFIG_DBGTERM_OUT_DEVICE_INIT_MODE) != __DEV_OK)
		{
			if (in)
			{
				__deviceClose(in);
				__deviceDeinit(in);
			}
			return;
		}

		if (__deviceOpen(out, __CONFIG_DBGTERM_OUT_DEVICE_OPEN_MODE) != __DEV_OK)
		{
			__deviceDeinit(out);
			if (in)
			{
				__deviceClose(in);
				__deviceDeinit(in);
			}
			return;
		}

		__deviceIOCtl(out, __IOCTL_SETTXTIMEOUT, 20, __NULL, 0);
		__deviceIOCtl(out, __IOCTL_SETRXTIMEOUT, 20, __NULL, 0);
	}

	__dbgTerminal = __terminalCreate("dbgterm", in, out, __CONFIG_DBGTERM_LINE_LENGTH, __NULL,
									 __TERMINAL_ECHO_ENABLED | __TERMINAL_DEBUG_COMMANDS);

	if (__dbgTerminal) __terminalStart(__dbgTerminal);
}

/*!
 * @brief Creates a command and adds it to the debug terminal.
 *
 * @param	cmd		Null-terminated string representing the command.
 * @param	func	Function to be called when the command is received.
 *
 * @return Nothing.
 */
__VOID __dbgAddCommand(__PSTRING cmd, __TERMINALRXCMD* func)
{
	__terminalCreateCommand(__dbgTerminal, cmd, func);
}

/*!
 * @brief Returns a pointer to the debug terminal
 *
 * @return A pointer to the __TERMINAL structure representing the Debug Terminal.
 */
__PTERMINAL __dbgGetSysTermPtr(__VOID)
{
	return __dbgTerminal;
}
/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

#endif /* __CONFIG_DBGTERM_ENABLED */


