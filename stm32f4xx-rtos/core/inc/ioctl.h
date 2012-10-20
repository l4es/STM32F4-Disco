/***************************************************************************
 * ioctl.h
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

/*
 * Main file for IOCTL codes definitions.
 * New IOCTL codes should be appended at the end.
 * IOCTL code from __deviceIOCtl() function is a 32-bit value
 * so there are 0xFFFFFFFF values left to declare.
 * In many cases a single ioctl code should be reused in different
 * drivers.
 */

#define	__IOCTL_SETSPEED			0		/*!< @brief Set device speed */
#define	__IOCTL_GETSPEED			1		/*!< @brief Get device speed */
#define	__IOCTL_SETBITLENGHT		2		/*!< @brief Set bits */
#define	__IOCTL_GETBITLENGHT		3		/*!< @brief Get bits */
#define	__IOCTL_SETPARITY			4		/*!< @brief Set parity */
#define	__IOCTL_GETPARITY			5		/*!< @brief Get parity */
#define	__IOCTL_SETSTOPBITS			6		/*!< @brief Set stop bit */
#define	__IOCTL_GETSTOPBITS			7		/*!< @brief Get stop bit */
#define __IOCTL_SETFLOWCONTROL		8		/*!< @brief Set flow control */
#define __IOCTL_GETFLOWCONTROL		9		/*!< @brief Get flow control */
#define	__IOCTL_SETRXTIMEOUT		10		/*!< @brief Set RX timeout */
#define	__IOCTL_GETRXTIMEOUT		11		/*!< @brief Get RX timeout */
#define	__IOCTL_SETTXTIMEOUT		12		/*!< @brief Set TX timeout */
#define	__IOCTL_GETTXTIMEOUT		13		/*!< @brief Get TX timeout */
#define __IOCTL_ECHO_ONOFF			14		/*!< @brief ECHO enable/disable */
#define __IOCTL_ECHO				15		/*!< @brief Get ECHO */
#define	__IOCTL_SETRTS				16		/*!< @brief Set RTS */
#define	__IOCTL_SETDTR				17		/*!< @brief Set DTR */
#define __IOCTL_ASSERT_CS			18		/*!< @brief Enable Chip Select generation */
#define __IOCTL_SET_CS				19		/*!< @brief Set Chip Select */
#define __IOCTL_MEDIA_AVAILABLE		20		/*!< @brief MMC/SD available */
#define __IOCTL_WRITE_PROTECTED		21		/*!< @brief Media is write protected */
#define __IOCTL_SET_SECTOR			22		/*!< @brief Set sector */
#define __IOCTL_GET_SECTOR			23		/*!< @brief Get sector */
#define __IOCTL_GET_POWER_STATUS	24		/*!< @brief Get device's power status */
#define __IOCTL_SET_POWER_STATUS	25		/*!< @brief Set device's power status */
#define __IOCTL_GET_SECTOR_COUNT	26 		/*!< @brief Retrieve sector count */
#define __IOCTL_GET_BLOCK_SIZE		27		/*!< @brief Get Block size */
#define __IOCTL_GET_CARD_TYPE		28		/*!< @brief Get Card type */
#define __IOCTL_GET_CSD				29		/*!< @brief Get Card Specific Data */
#define __IOCTL_GET_CID				30 		/*!< @brief Get CID */
#define __IOCTL_GET_OCR				31 		/*!< @brief Get OCR */
#define __IOCTL_GET_SDSTAT			32 		/*!< @brief Get SD status */
#define __IOCTL_GET_FIX_FLAG		33		/*!< @brief For GPS, returns the FIX flag */
#define __IOCTL_TERMINAL_OUTPUT		34		/*!< @brief Enables/disables terminal output */
#define __IOCTL_SET_LINE			35		/*!< @brief Sets the cursor at a given line (display) */
#define __IOCTL_SET_COLUMN			36		/*!< @brief Sets the cursor at a given column (display) */
#define __IOCTL_CLEAR_LINE			37		/*!< @brief Clears a line (display) */
#define __IOCTL_CLEAR_SCREEN		38		/*!< @brief Clears the screen (display) */
#define __IOCTL_SET_REMOTE_ADDR		39		/*!< @brief Set remote address */
#define __IOCTL_SET_REMOTE_SUBADDR	40		/*!< @brief Set remote sub-address */
#define __IOCTL_SET_OWN_ADDR		41		/*!< @brief Set own address (i2c slave) */
#define __IOCTL_SET_MODE			42		/*!< @brief Set working mode */
#define __IOCTL_SET_BLOCK			43		/*!< @brief Set block */
#define __IOCTL_GET_BLOCK			44		/*!< @brief Get block */
#define __IOCTL_SET_ADDRESS			45		/*!< @brief Set address */
#define __IOCTL_GET_ADDRESS			46		/*!< @brief Get address */
#define __IOCTL_SET_PAGE			47		/*!< @brief Set page */
#define __IOCTL_GET_PAGE			48		/*!< @brief Get page */
#define __IOCTL_SET_MAX_PAGES		49		/*!< @brief Set maximum quantity of pages */
#define __IOCTL_GET_MAX_PAGES		50		/*!< @brief Get maximum quantity of pages */
#define __IOCTL_GET_FRAME_SIZE		51		/*!< @brief Get next frame size (network interface) */
#define __IOCTL_GET_MAC_ADDRESS		52		/*!< @brief Get 6 bytes length MAC address */
#define __IOCTL_GET_LINK_STATUS		53		/*!< @brief Get the Ethernet link status */
#define __IOCTL_QUERY_LINK_STATUS	54		/*!< @brief Query the PHY for the Ethernet link status */
#define __IOCTL_SET_RXBUF_SIZE		55		/*!< @brief Set RX buffer size */
#define __IOCTL_SET_TXBUF_SIZE		56		/*!< @brief Set TX buffer size */
#define __IOCTL_GET_RXBUF_SIZE		57		/*!< @brief Get RX buffer size */
#define __IOCTL_GET_TXBUF_SIZE		58		/*!< @brief Get TX buffer size */

