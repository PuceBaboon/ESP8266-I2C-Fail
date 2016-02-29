/*
 * $Id: u_get_dstemp.c,v 1.8 2015/08/07 04:22:20 anoncvs Exp $
 */
#include "ets_sys.h"
#include "osapi.h"
#include "debug.h"
#include "mqtt.h"
#include "driver/ds3231.h"
#include "u_ds3231.h"
#include "u_time.h"

#define	DEBUG 1		// Enable "INFO" messages.
#define BUFSZ 10
int8_t		TemprBuf[BUFSZ];
const int8_t	*tBuf = (int8_t *) &TemprBuf;

void ICACHE_FLASH_ATTR ds3231_get_temp(void *arg) {
	bool		f = false;	// Flag holder.
	int8_t		i, my_tempMSB = 0xFF, my_tempLSB = 0xFF;
	MQTT_Client	mqttClient;

#ifdef DEBUG
	INFO("Forcing new temperature conversion.\n");
#endif
	if (! ds3231_getFlag(DS3231_ADDR_STATUS, DS3231_STAT_BUSY, &f)) {
		INFO("Failed to read BUSY flag!\n");
		return;
	}
	if (f == true) {
#ifdef DEBUG
		INFO("Conversion already currently in progress.\n");
#endif
	} else {
		// Set the CONV flag to start the temperature conversion.
		if (! ds3231_setFlag(DS3231_ADDR_CONTROL, DS3231_CTRL_TEMPCONV, DS3231_SET)) {
			INFO("Failed to set CONV flag!\n");
			return;
		}
		// Now wait for the CONV flag to be reset (signalling
		// the end of conversion).
		do {
			os_delay_us(1000);		//  This is to allow system processes to run. Do not change.
			if (! ds3231_getFlag(DS3231_ADDR_CONTROL, DS3231_CTRL_TEMPCONV, &f)) {
				INFO("Failed to get CONV flag state!\n");
				return;
			}
#ifdef DEBUG
			INFO(".");
#endif
		} while (f == true);
#ifdef DEBUG
		INFO("\n");
#endif
	}

#ifdef DEBUG
	INFO("Fetching (two-byte) temperature from DS3231.\n");
#endif
	if (! ds3231_getTempBytes(&my_tempMSB, &my_tempLSB)) {
		INFO("Fetch temp failed!\n");
		return;
	}
	os_delay_us(1000);		//  This is to allow system processes to run. Do not change.

#ifdef DEBUG
	INFO("%3d.%02dC\n", my_tempMSB, my_tempLSB);
#endif

	// Publish the RTC temperature to MQTT.
	os_sprintf(tBuf, "%d.%02dC", my_tempMSB, my_tempLSB);
	MQTT_Publish(&mqttClient, TOPIC2, tBuf, strlen(tBuf), 2, 0);
	return;
}


void ICACHE_FLASH_ATTR ds3231_pub_temp(void *arg) 
{
	int8_t		my_temptr, i, my_tempMSB = 0xFF, my_tempLSB = 0xFF;
	int8_t		my_mdata[0x10];
	uint16		my_maddr = 0x20;
	uint16		my_mlen = 0x0A;
	struct tm	gen_time;	// Generic time structure.  Re-use.
	struct tm_struc	loc_time;	// Local time structure (1970 Epoch).

//	(void) ds3231_get_temp((void *) 0);

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

	/*
	 * Set the RTC alarm.
	 * What we want to do here is set the seconds alarm to the current time
	 * plus ten seconds and then re-enable it (still using LED output
	 * indicator), to verify that this is a viable method.  We will re-use
	 * the gen_time tm struct to temporarily hold the current time readout
	 * while we check and recalculate the new seconds setting.
	 */
	int8_t		a1secs;
	if (! ds3231_getTime(&gen_time)) {
		INFO("DS3231 fetch time failed!\n");
		return;
	}

	/*
	int8_t		alarm_f = 0;	// Alarm status flags holder.
	if (!ds3231_getAlarmFlags(&alarm_f)) {
		INFO("Failed to get alarm flag status!\n");
		return;
	} else {
		if (alarm_f != DS3231_ALARM_NONE) {
			INFO("An alarm is currently active: %d\n", alarm_f);
			switch(alarm_f) {
				case DS3231_ALARM_BOTH:	INFO("Both");
							break;
				case DS3231_ALARM_1:	INFO("A1");
							break;
				case DS3231_ALARM_2:	INFO("A2");
							break;
				default:		INFO("Unknown alarm flag output!");
			}
			INFO("\n");
		} else {
			return;
		}
	}
	*/

	/*
	 * Set the next scheduled alarm by adding T_DELAY minutes
	 * (from include/user_config.h) to the current minute (from
	 * the time value we just retrieved from the DS3231).
	 */
	gen_time.tm_min += T_DELAY;
	if (gen_time.tm_min >= 60) { 	// Minute roll-over.
		gen_time.tm_min -= 60;
	}
#ifdef DEBUG
	INFO("Updated A2 = %d\n", gen_time.tm_min);
#endif
	gen_time.tm_hour = 0;
	gen_time.tm_mday = 0;
	gen_time.tm_mon = 0;
	gen_time.tm_year = 0;
	gen_time.tm_wday = 0;
	gen_time.tm_yday = 0;
	gen_time.tm_isdst = 0;

	/*
	 * Now reset the current alarm flags, load the new alarm-2 time
	 * and enable the interrupt function on the DS3231 SQW/INT pin.
	 */
	ds3231_disableAlarmInts(DS3231_ALARM_BOTH);	// Specifically disable both alarm interrupts.
	ds3231_disable32khz();				// Ensure 32kHz disabled.
	ds3231_disableSquarewave();			// Disable SqWv and enable alarm interrupts.
	ds3231_clearAlarmFlags(DS3231_ALARM_BOTH);	// Clear flags for both alarms.
	ds3231_setAlarm(DS3231_ALARM_2, 0, 0, &gen_time, DS3231_ALARM2_MATCH_MIN);
	ds3231_enableAlarmInts(DS3231_ALARM_2);		// Enable interrupt for alarm-2.

#ifdef DEBUG
	/*
	 * Check the current alarm status.
	 */
	int8_t		alarm_f = 0;	// Alarm status flags holder.
	if (!ds3231_getAlarmFlags(&alarm_f)) {
		INFO("Failed to get alarm flag status!\n");
		return;
	} else {
		if (alarm_f != DS3231_ALARM_NONE) {
			INFO("Shit! An alarm is currently active: %d\n", alarm_f);
		} else {
			INFO("Relax! No alarms active right now.\n");
		}
	}
	
	INFO("A1 now: ");
	for (i=0; i<=3; i++) {
		disp_r = 0;
		ds3231_getFlag(DS3231_ADDR_ALARM1 + i, 0xFF, &disp_r);
		INFO("%02X ", (unsigned)disp_r);
	}
	INFO("\n");
	INFO("A2 now: ");
	for (i=0; i<=2; i++) {
		disp_r = 0;
		ds3231_getFlag(DS3231_ADDR_ALARM2 + i, 0xFF, &disp_r);
		INFO("%02X ",(unsigned) disp_r);
	}
	INFO("\n");
#endif
}
