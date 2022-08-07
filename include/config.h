#ifndef _CONFIG_H
#define _CONFIG_H

#include <IPAddress.h>

#define BAUD_RATE 9600

// NTP stuff
#define NTP_SERVER "pool.ntp.org"       // Or time.nist.gov
#define NTP_TZ_OFFSET -4                // Eastern Time (adds -1 for DST)

// Task intervals
#define DHCP_RENEW_INTERVAL_MINUTES 5   // TODO Make this a more sane value (ie. 1 hour)
#define NTP_UPDATE_INTERVAL_MINUTES 5   // TODO Make this a more sane value (ie. 24 hours)

// Hardware RTC stuff
#define CLOCK_TYPE_DS3231 0
#define CLOCK_TYPE_PCF8523 1
#define CLOCK_TYPE_ACTIVE CLOCK_TYPE_PCF8523

#define DEVICE_NAME "RC2014HAB"
#define MQTT_BROKER "192.168.0.104"
#define MQTT_PORT 8883
#define MQTT_USERNAME "cygarage"
#define MQTT_PASSWORD "gonchar"

#endif