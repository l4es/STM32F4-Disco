/***************************************************************************
 * plat_rtc.c
 * (C) 2011 Ivan Meleca
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

#include "plat_rtc.h"
#include <common/inc/mem.h>
#include <stm32f4xx_rtc.h>
#include <stm32f4xx_pwr.h>

#if __CONFIG_COMPILE_RTC

/** @addtogroup RTC_Stm32
  * @{
  */

/** @defgroup RTC_Stm32_PrivateVariables Private variables
  * @{
  */

/**
  * @}
  */

/** @defgroup RTC_Stm32_Functions Functions
  * @{
  */

/*!
 * @brief RTC initialization for the STM32.
 *
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __rtcPlatInit(__VOID)
{
	RTC_InitTypeDef		RTC_InitStructure;

	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* LSI used as RTC source clock */
	/* The RTC Clock may varies due to LSI frequency dispersion. */
	  /* Enable the LSI OSC */
	  RCC_LSICmd(ENABLE);

	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {}

	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();

	/* Calendar Configuration with LSI supposed at 32KHz - 0x7FFFh */
	RTC_InitStructure.RTC_AsynchPrediv	= 0x7F;
	RTC_InitStructure.RTC_SynchPrediv	= 0xFF; /* (32KHz / 128) - 1 = 0xFF*/
	RTC_InitStructure.RTC_HourFormat 	= RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);

	//PWR_PVDLevelConfig(PWR_PVDLevel_6);
	//PWR_PVDCmd(ENABLE);

	return __TRUE;
}

/*!
 * @brief Converts the STM32 counter to a __DATETIME structure format.
 *
 * @param dt	Pointer to a __DATETIME structure to receive the current date and time.
 *
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __rtcPlatGetDateTime(__PDATETIME dt)
{
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef	RTC_DateStructure;

	__memSet(dt,0,sizeof(__DATETIME));

	// Time
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
	dt->sec		= RTC_TimeStructure.RTC_Seconds;
	dt->min		= RTC_TimeStructure.RTC_Minutes;
	dt->hour	= RTC_TimeStructure.RTC_Hours;

	/*	Date */
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
	dt->dayofweek 	= RTC_DateStructure.RTC_WeekDay;
	dt->day	 		= RTC_DateStructure.RTC_Date;
	dt->month		= RTC_DateStructure.RTC_Month;
	dt->year 		= RTC_DateStructure.RTC_Year;

	return __TRUE;
}

/*!
 * @brief Converts a __DATETIME structure to the STM32 counter format.
 *
 * @param dt	Pointer to a __DATETIME structure containing the date and time.
 *
 * @return __TRUE on success, otherwise __FALSE.
 */
__BOOL __rtcPlatSetDateTime(__PDATETIME dt)
{
	__DATETIME datetime;
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef	RTC_DateStructure;

	__memCpy(&datetime, dt, sizeof(__DATETIME));

	// Date
	RTC_DateStructure.RTC_Year		= datetime.year;
	RTC_DateStructure.RTC_Month		= datetime.month;
	RTC_DateStructure.RTC_Date		= datetime.day;
	RTC_DateStructure.RTC_WeekDay	= datetime.dayofweek;
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);

	// Time
	RTC_TimeStructure.RTC_Hours		= datetime.hour;
	RTC_TimeStructure.RTC_Minutes	= datetime.min;
	RTC_TimeStructure.RTC_Seconds	= datetime.sec;
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);

	return __TRUE;
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CONFIG_COMPILE_RTC */
