/*
 * $Id: u_ds3231.c,v 1.13 2015/07/26 00:25:52 anoncvs Exp $
 */
#include "ets_sys.h"
#include "osapi.h"
#include "debug.h"
#include "driver/ds3231.h"
#include "u_ds3231.h"
#include "u_time.h"

int8_t		start_f = 1;	// Global start flag.
struct tm	GenTime;	// Generic time structure.  Re-use.

/*
 * Set the DS3231 time from the MQTT (server) published timestamp.
 * Checks the RTC idea of the current time against the (NTP controlled)
 * MQTT server timestamp.  If off by more (or less) than LEEWAY_SECS
 * then the RTC will be updated with the current time.
 * This is called from the MQTT subscription callback, so it should
 * never be more than a fraction of a second out from the actual
 * time.
 * LEEWAY_SECS needs to be some reasonable, multiple second value.
 * The default is currently "5".
 *
 * Arg "mqtt_tstamp" is a pointer to the integer seconds count value
 * of the MQTT timestamp just received.
 * Arg "s_time" is a pointer to a tm_struc struct, pre-populated 
 * with the values from the "mqtt_tstamp".  [THIS IS A BAD IDEA!!]
 *
 */
void ICACHE_FLASH_ATTR ds3231_chk_set(int *mqtt_tstamp, struct tm_struc *s_time) {
	int			timenow = 0;
	struct tm_struc	converted;
#ifdef DEBUG
	INFO("In ds3231_chk_set()\nFetching time from DS3231.\n");
#endif
	if (! ds3231_getTime(&GenTime)) {
		INFO("DS3231 fetch time failed!\n");
		return;
	} else {
		ds3231_tm_conv(&GenTime, &converted);
		timenow = tm_to_timet(&converted);
		if (timenow == 0) {
			INFO("Call to tm_to_timet() failed!\n");
			return;
		}
	}

#ifdef DEBUG
	INFO("MQTT: %d\n RTC: %d\n", *mqtt_tstamp, timenow);
#endif

	if ((timenow >= (*mqtt_tstamp + 5)) | (timenow <= (*mqtt_tstamp - 5))) {
			
#ifdef DEBUG
		INFO("RTC clock wrong.  Resetting from MQTT timestamp.\n");
		int tdiff = *mqtt_tstamp - timenow;
		timet_to_tm(tdiff, &converted);
		INFO("---------------------------------------------------------------------------------\n");
		INFO("Diff is:-\tYr: %d   Mn: %d   ", converted.Year, converted.Month);
		INFO("Md: %d   Hr: %d   ", converted.Day, converted.Hour);
		INFO("Mn: %d   Sc: %d\n", converted.Minute, converted.Second);
		INFO("---------------------------------------------------------------------------------\n");
#endif
		ds3231_tstamp_conv(s_time, &GenTime);
		ds3231_setTime(&GenTime);
#ifdef DEBUG
		INFO("Set RTC time from MQTT timestamp.\n");
#endif
	}
}


/*
 * Convert the contents of a DS3231 tm structure to an epoch structure.
 */
void ICACHE_FLASH_ATTR ds3231_tm_conv(struct tm *rtc_tm, struct tm_struc *conv) {
	uint8_t		i = 0;

	rtc_tm->tm_year -= 70;	/* Convert from 1900 offset to 1970 offset.

	/* 
	 * Add the number of elapsed days for the given year. 
	 * Months start from 0 in standard, time.h struct tm.
	 */
 	for (i = 0; i < rtc_tm->tm_mon; i++) {
 		if ( (i == 1) && LEAP_YEAR(rtc_tm->tm_year)) {	// February.
 			rtc_tm->tm_yday += 29;
 		} else {
 			rtc_tm->tm_yday += monthDays[i-1];			// The "monthDay" array starts from 0.
 		}
 	}

	// Copy the contents of the RTC struct into a temporary tm_struc struct,
	conv->Second 	= rtc_tm->tm_sec;
	conv->Minute	= rtc_tm->tm_min;
	conv->Hour	= rtc_tm->tm_hour;
	conv->Wday	= rtc_tm->tm_wday + 1;
	conv->Day	= rtc_tm->tm_mday;
	conv->Month	= rtc_tm->tm_mon + 1;
	conv->Year	= rtc_tm->tm_year;
}

/*
 * And nowthe opposite... Convert a tm_struc (epoch) struct into a DS3231 tm struct
 * to use in setting the RTC.
 */
void ICACHE_FLASH_ATTR ds3231_tstamp_conv(struct tm_struc *tstmp, struct tm *rtc_tm) {
	rtc_tm->tm_year		= tmYearToY2k(tstmp->Year) + 100;
	rtc_tm->tm_mon		= tstmp->Month - 1;
	rtc_tm->tm_mday		= tstmp->Day;
	rtc_tm->tm_wday		= tstmp->Wday - 1;
	rtc_tm->tm_hour		= tstmp->Hour;
	rtc_tm->tm_min		= tstmp->Minute;
	rtc_tm->tm_sec		= tstmp->Second;
	rtc_tm->tm_yday		= 0;	// Not used.
	rtc_tm->tm_isdst	= 0;	// Used, but not in Japan.
}


void ICACHE_FLASH_ATTR ds3231_cb(void *arg) 
{
	int8_t		my_temptr, i, my_tempMSB = 0xFF, my_tempLSB = 0xFF;
	int8_t		my_mdata[0x10];
	uint16		my_maddr = 0x20;
	uint16		my_mlen = 0x0A;

	// system_print_meminfo();

	if (start_f == 1) {
		ds3231_disable32khz();		// Ensure 32kHz disabled.
		start_f = 0;	// Reset start flag to show we're now live.
	}

#ifdef DEBUG
	int8_t		disp_r = 0;
	INFO("A1: ");
	for (i=0; i<=3; i++) {
		disp_r = 0;
		ds3231_getFlag(DS3231_ADDR_ALARM1 + i, 0xFF, &disp_r);
		INFO("%02X ", (unsigned)disp_r);
	}
	INFO("\n");
	INFO("A2: ");
	for (i=0; i<=2; i++) {
		disp_r = 0;
		ds3231_getFlag(DS3231_ADDR_ALARM2 + i, 0xFF, &disp_r);
		INFO("%02X ", (unsigned)disp_r);
	}
	INFO("\n");
	ds3231_getFlag(DS3231_ADDR_CONTROL, 0xFF, &disp_r);
	INFO("CTRL: %02X\n", disp_r);
	ds3231_getFlag(DS3231_ADDR_STATUS, 0xFF, &disp_r);
	INFO("STAT: %02X\n", disp_r);
#endif
	ds3231_setAlarm(DS3231_ALARM_NONE, 0, 0, 0, 0);	// Clear both alarm settings.
#ifdef DEGUG
	INFO("Fetching time from DS3231.\n");
#endif
	if (! ds3231_getTime(&GenTime)) {
		INFO("DS3231 fetch time failed!\n");
		return;
	}
#ifdef DEGUG
	INFO("Fetch complete.\n");
	INFO(">>>   Y%d:M%d:D%d ", GenTime.tm_year, GenTime.tm_mon, GenTime.tm_mday);
	INFO("H%d:M%d:S%d\n", GenTime.tm_hour, GenTime.tm_min, GenTime.tm_sec);
#endif
}
