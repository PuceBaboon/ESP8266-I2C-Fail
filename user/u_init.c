
/*
 *
 * $Id: u_init.c,v 1.17 2015/08/06 15:12:15 anoncvs Exp $
 *
 * Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of Redis nor the names of its contributors may be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "u_init.h"
#include "u_time.h"
#include <time.h>

MQTT_Client	mqttClient;

// Connect call-back.
void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}

void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Connected\r\n");
	MQTT_Subscribe(client, TOPIC1, 0);
	MQTT_Subscribe(client, TOPIC2, 1);
	MQTT_Subscribe(client, TOPIC3, 2);

	ds3231_get_temp((void *) 0);
}

void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Disconnected\r\n");
}

void ICACHE_FLASH_ATTR mqttPublishedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Published\r\n");
}

void ICACHE_FLASH_ATTR mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	char *topicBuf = (char*)os_zalloc(topic_len+1),
			*dataBuf = (char*)os_zalloc(data_len+1);

	MQTT_Client* client = (MQTT_Client*)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

#define DEBUG 1
#ifdef DEBUG
	INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
#endif
	if (strncmp(topicBuf, TOPIC1, strlen(TOPIC1)) == 0) {
#ifdef DEBUG
		INFO("Topic#1 - ESP Status.\n");
#endif
	} else if (strncmp(topicBuf, TOPIC2, strlen(TOPIC2)) == 0) {
#ifdef DEBUG
		INFO("Topic#2 - ESP Temperature.\n");
#endif
	} else if (strncmp(topicBuf, TOPIC3, strlen(TOPIC3)) == 0) {
#ifdef DEBUG
		INFO("Topic#3 - Epoch Timestamp.\n");
#endif
		int	tstamp = atoi(dataBuf);
		struct tm_struc	e_tmel;
		timet_to_tm((time_t) tstamp, &e_tmel);
		ds3231_chk_set(&tstamp, &e_tmel);
#ifdef DEBUG
		INFO("Epoch kara:-\n\tYear:\t%04d\n\tMonth:\t%02d\n", e_tmel.Year + 1970, e_tmel.Month);
		INFO("\tWDay:\t%d\n\tHour:\t%02d\n", e_tmel.Wday, e_tmel.Hour);
		INFO("\tMin:\t%02d\n\tSec:\t%02d\n", e_tmel.Minute, e_tmel.Second);
#endif
	} else {
		INFO("Unknown topic received\n");
	}
	os_free(topicBuf);
	os_free(dataBuf);
}

void ICACHE_FLASH_ATTR u_mqtt_init()
{
	CFG_Load();

	/*
	 * Original, old call with SSID and Passwd included.
	 * WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);
	 *
	 * NOTE:- Our WiFi-Connect callback calls the MQTT_Connect function,
	 *        which in turn will call the mqttConnectCb() function when
	 *        communications with the MQTT servcer are established.
	 */
	WIFI_Connect(wifiConnectCb);

	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
	MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);
}
