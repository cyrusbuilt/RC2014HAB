#ifndef _TELEMETRY_H
#define _TELEMETRY_H

#include <Arduino.h>

enum class DeviceCommand : uint8_t {
	UNKNOWN = 0,
	GARAGE_ACTIVATE = 1,
	LIGHT_1_ON = 2,
	LIGHT_1_OFF = 3,
	LIGHT_2_ON = 4,
	LIGHT_2_OFF = 5,
	LIGHT_3_ON = 6,
	LIGHT_3_OFF = 7,
	LIGHT_4_ON = 8,
	LIGHT_4_OFF = 9,
	LIGHT_5_ON = 10,
	LIGHT_5_OFF = 11,
	DISABLE = 255
};

enum class DeviceStatus : uint8_t {
	UNKNOWN = 0,
	GARAGE_OPEN = 1,
	GARAGE_CLOSED = 2,
	GARAGE_AJAR = 3,
	GARAGE_DISABLED = 4,
	CL_LIGHT1_ON = 5,
	CL_LIGHT1_OFF = 6,
	DISABLED = 255
};

/**
 * Helper class providing static telemetry helper methods.
 */
class TelemetryHelper
{
public:
	static String getMqttStateDesc(int state);
};

#endif