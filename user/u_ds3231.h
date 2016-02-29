/*
 * $Id: u_ds3231.h,v 1.5 2015/07/23 10:40:14 anoncvs Exp $
 */
#ifndef __U_DS3231_H__
#define __U_DS3231_H__

#include "u_time.h"

void ICACHE_FLASH_ATTR ds3231_cb(void *arg);
void ICACHE_FLASH_ATTR ds3231_tm_conv(struct tm *rtc_tm, struct tm_struc *conv);
void ICACHE_FLASH_ATTR ds3231_tstamp_conv(struct tm_struc *tstmp, struct tm *rtc_tm);


#endif
