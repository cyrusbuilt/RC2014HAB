#ifndef _CYLIGHTS_H
#define _CYLIGHTS_H

#include <Arduino.h>

#define CYLIGHTS_DEVICEID "CYLIGHTS"
#define TOPIC_CYLIGHTS_STATUS "cylights/status"
#define TOPIC_CYLIGHTS_CONTROL "cylights/control"

enum class CyLightsState: uint8_t {
	BOOTING = 0,
	NORMAL = 1,
	UPDATING = 2,
	RECONNECTING = 3,
	DISABLED = 4,
	CHECKIN = 5
};

enum class CyLightsControlCommand: uint8_t {
	/**
	 * Disable the system. Causes the system to ignore any further incoming
	 * control commands until the "ENABLE" command is received.
	 */
	DISABLE = 0,

	/**
	 * Enables the system if it is currently disabled.
	 */
	ENABLE = 1,

	/**
	 * Reboot the system.
	 */
	REBOOT = 2,

	/**
	 * Get the current status of the system and all controlled outlets.
	 */
	REQUEST_STATUS = 3,

	/**
	 * Turn light (outlet) 1 on.
	 */
	LIGHT1_ON = 4,

	/**
	 * Turn light (outlet) 1 off.
	 */
	LIGHT1_OFF = 5,

	/**
	 * Turn light (outlet) 2 on.
	 */
	LIGHT2_ON = 6,

	/**
	 * Turn light (outlet) 2 off.
	 */
	LIGHT2_OFF = 7,

	/**
	 * Turn light (outlet) 3 on.
	 */
	LIGHT3_ON = 8,

	/**
	 * Turn light (outlet) 3 off.
	 */
	LIGHT3_OFF = 9,

	/**
	 * Turn light (outlet) 4 on.
	 */
	LIGHT4_ON = 10,

	/**
	 * Turn light (outlet) 4 off.
	 */
	LIGHT4_OFF = 11,

	/**
	 * Turn light (outet) 5 on.
	 */
	LIGHT5_ON = 12,

	/**
	 * Turn light (outlet) 5 off.
	 */
	LIGHT5_OFF = 13,

	/**
	 * Turn ALL lights (outlets) on.
	 */
	ALL_ON = 14,

	/**
	 * Turn ALL lights (outlets) off.
	 */
	ALL_OFF = 15,

	/**
	 * Reset the onboard I/O controller.
	 */
	IO_RESET = 16
};

#endif