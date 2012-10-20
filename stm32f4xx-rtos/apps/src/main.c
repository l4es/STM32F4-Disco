#include <system.h>
#include <thread.h>
#include <dbgterm.h>
#include <rtc.h>
#include <string.h>

__STATIC __PSTRING __appDay[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Mon"};

__STATIC __VOID appTerminalCmd(__PSTRING cmd, __PSTRING params)
{
	__DATETIME dt;

	if (__strnCmpNoCase(cmd, "set", 3) == 0)
	{

		dt.day = 23;
		dt.month = 12;
		dt.year = 2011;

		dt.hour = 20;
		dt.min = 47;
		dt.sec = 10;

		dt.dayofweek = 0;

		__rtcSetDateTime(&dt);
	}
}

/*
 * Thread 1
 */
__VOID threadTest1(__VOID)
{
	__DATETIME dt;

	for (;;)
	{
		__rtcGetDateTime(&dt);

		DBGMSG(__TRUE, ("Datetime: %s %02lu.%02lu.%04lu %02lu:%02lu:%02lu",	__appDay[dt.dayofweek],
																			dt.day,
																			dt.month,
																			dt.year,
																			dt.hour,
																			dt.min,
																			dt.sec));

		__threadSleep(2000);
	}
}

/*!
 * @brief Application entry point.
 */
__VOID appEntry(__VOID)
{
	__dbgAddCommand("set", appTerminalCmd);
	__threadCreate("test1", threadTest1, 80, 512, 1, __NULL);
}

/*!
 * @brief Program entry point.
 */
int main(void)
{
	__systemInit(appEntry);
	return 0;
}
