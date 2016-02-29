#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

// #define CFG_HOLDER	0x00FF55A4	/* Change this value to load default configurations */
// #define CFG_LOCATION	0x3C		/* Please don't change or if you know what you doing */
#define CFG_HOLDER	0x00FF55A8	/* Change this value to load default configurations */
#define CFG_LOCATION	0x7A		/* 0x78 - Please don't change if you don't know what you doing */
#define CLIENT_SSL_ENABLE

// #define PROTOCOL_NAMEv31	/*MQTT version 3.1 compatible with Mosquitto v0.15*/
#define PROTOCOL_NAMEv311			/*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/

//----------------------------------------------

/*DEFAULT CONFIGURATIONS*/

#define MQTT_HOST	"192.168.1.1"
#define MQTT_PORT	1883
#define MQTT_BUF_SIZE	1024
#define MQTT_KEEPALIVE	120	// In seconds.

#define MQTT_CLIENT_ID	"Puce_%08X"
#define MQTT_USER	"Puce_USER"
#define MQTT_PASS	"Puce_PASS"

#define STA_MODE	"static"
#define STA_IP		"192.168.1.120"
#define STA_MASK	"255.255.255.0"
#define STA_GW		"192.168.1.1"
#define STA_SSID	"!! YOUR AP SSID !!"
#define STA_PASS	"!! YOUR SSID PASSWD !!"
#define STA_TYPE	AUTH_WPA2_PSK

#define MQTT_RECONNECT_TIMEOUT	5	// In Seconds.

#define CLIENT_SSL_ENABLE
#define DEFAULT_SECURITY	0

#define QUEUE_BUFFER_SIZE 	1024

/* I2C GPIO Defines. */
#define I2C_MASTER_SDA_GPIO	2
#define I2C_MASTER_SCL_GPIO	14

/* OW_GPIO defines the pin used for the one-wire bus */
#ifndef OW_GPIO
#define OW_GPIO		14
#endif

#define T_DELAY		10	// Minutes between temperature samples.

#define TOPIC1		"ESP/status"
#define TOPIC2		"ESP/temperature"
#define TOPIC3		"TIME/Epoch"
#endif
//
//----------------------------------------------
