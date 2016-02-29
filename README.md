# ESP8266-I2C-Fail
An abandoned attempt at creating a DS3231/ESP8266/I2C project in standard "C" using the Espressif SDK

##DS3231/ESP8266 I2C (Abandoned)

Big letters at the top, "Abandoned".

This code is only here because of requests from readers of the esp8266-hints blog (https://esp8266hints.wordpress.com/2015/06/04/sdk-i2c-code-todays-duh-story) who wanted to see real-world examples in all of their inglorious detail.

Since trying to get this version to work I have moved on to using the Arduino-IDE for ESP8266 development work.  I'm *not* an Arduino person (don't own one, haven't stolen one, yet) and I'm not terribly keen on graphical editors either (I'll take
"vi" and command-line tools any day), but the Arduino-IDE for the ESP8266 __just works__.  No faffing about with eagle.v6 files in the SDK to try and squeeze your code into the correct segment of memory, no trying to remember how the GPIO pins
are mapped this week, no going back (yet again!) into the updated SDK to find out what files Espressif (in their wisdom) haven't shipped with this version, or any of the other myriad of crap which seems to bog down even the simplest of
projects for hours (or days) on end.  If you haven't guessed already, I'm a convert.  I still won't be buying an Arduino, but the folks who wrote and maintain the ESP8266 core for Arduino (https://github.com/esp8266/Arduino) have my undying
gratitude.

Having said all of that, this repository contains the older, plain old "C" + Makefile version of the code.  Please understand that it is here as an example and nothing more.  I won't be developing it further.  I won't be accepting pull requests.  I won't be doing *anything* further with this branch at all.  It's dead, Jim.  Nailed to the perch.  Floating down the Styx.

This repository contains code from others far more talented than me.  The original project used Tuan PM's MQTT for the ESP8266 (https://github.com/tuanpmt/esp_mqtt) as well as drivers from various sources (Richard A. Burton -  https://github.com/raburton/esp8266/tree/master/drivers,  EADF -  https://github.com/eadf/esp8266_i2c_master,   Zarya -
https://github.com/zarya/esp8266_i2c_driver) and, of course, Espressif.  All of their code is inviolate and you should check the licences for each of the sources given above before re-using any code.

The included UnLicense licence refers to my hacked-up code, which is generally limited to the "user" directory.  The original project was based upon Tuan PM's mqtt directory structure, so even a couple of files in the user directory carry his copyright notice (u_init.c and u_main.c).  There are also notes in some of the other files giving attribution to sources of original code which I have heavily refactored (that's a nice way of saying "butchered") to fit in with this project.  You can safely assume that all mistakes, howlers, crap and general naughtiness are attributable to me.  Anything that actually works was written by someone else and you should go to the original source for pristine, un-molested code.

__m_epoch.sh__ - One thing worth noting is that the tiny shell script m_epoch.sh is meant to run on your MQTT server (not on the ESP8266!) to provide a Unix-style, epoch timestamp to all of the ESP8266'es (or anything else that's listening) on your network.  This allows (in this project) an uninitialized DS3231 module to have a fairly accurate time set by the ESP8266 when initially powered on.  Generally (in *any* project, not just this one), the timestamp can be used to provide an ESP8266 coming out of deep-sleep mode with a relatively accurate time (usually less than 10-seconds out) as soon as it connects to the MQTT server.  The server publishes the timestamps with the "retain" flag set, so the ESP8266 gets the last published value immediately on subscribing, rather than having to wait for the next publication slot.  I found this to be a useful alternative to adding an NTP client to the ESP8266 itself (especially as I was using the small-memory-footprint ESP-03 modules at the time).  For clarity, I should probably mention that this assumes that your MQTTserver *is* running NTP.


