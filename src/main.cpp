#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include "ArduinoJson.h"
#include "NTPClient.h"
#include "TaskScheduler.h"
#include "PubSubClient.h"
#include "RTClib.h"
#include "Telemetry.h"
#include "config.h"
#include "cygarage.h"
#include "cylights.h"

#define FIRMWARE_VERSION "1.0"

// Forward declarations
void onDhcpRenewTask();
void onNtpUpdateTask();
void onMqttMessage(char* topic, byte* payload, unsigned int length);

// Global vars
#if (CLOCK_TYPE_ACTIVE == CLOCK_TYPE_DS3231)
  RTC_DS3231 clock;
#elif (CLOCK_TYPE_ACTIVE == CLOCK_TYPE_PCF8523)
  RTC_PCF8523 clock;
#endif
Scheduler taskMan;
Task tDhcpRenew(DHCP_RENEW_INTERVAL_MINUTES * 60000, TASK_FOREVER, &onDhcpRenewTask);
Task tNtpUpdateTask(NTP_UPDATE_INTERVAL_MINUTES * 60000, TASK_FOREVER, &onNtpUpdateTask);
byte mac[6] = { 0xDE, 0xAD, 0xBE, 0xED, 0xFE, 0xEF };
EthernetUDP udpClient;
NTPClient timeClient(udpClient, NTP_SERVER, NTP_TZ_OFFSET);
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
const String controlChannels[1] = {
	TOPIC_GARAGE_CONTROL
};
const String statusChannels[1] = {
	TOPIC_GARAGE_STATUS
};
volatile DeviceStatus stat = DeviceStatus::DISABLED;
volatile DeviceCommand lastCmd = DeviceCommand::UNKNOWN;


/**
 * Determines whether or not the specified date parameters fall within US
 * Daylight Savings Time (DST).
 * @param hours Current hour of the day.
 * @param day Current day of the month.
 * @param month Current month of the year.
 * @param year Current year.
 * @param dow Current day of the week.
 * @return true if currently observing DST; Otherwise, false.
 */
bool isDST(int hours, int day, int month, int year, int dow) {
  if (day < 0 || month < 0 || year < 0) return false;
  if ((month >= 3) && (month <= 10)) { // March to October inclusive
    if ((month > 3) && (month < 10)) {
       return true;
    }
    if (month == 3) { // Have we passed the last Sunday of March, 1am UT ?
      if (day >= 25) {
         // When is the next sunday ?
         int dts = 6 - dow; // Number of days before the next sunday
         if (dts == 0) dts = 7; // We are really speaking of the next sunday, not the current one
         if ((day + dts) > 31) { // The next sunday is next month!
            if (dow != 6 || hours > 0) // We finally check that we are not on the day of the change before the time of the change
               return true;
         }
      }
    }
    if (month == 10) { // Have we passed the last Sunday of October 1am UT ?
      if (day >= 25) {
         // When is the next sunday ?
         int dts = 6 - dow; // Number of days before the next sunday
         if (dts == 0) dts = 7; // We are really speaking of the next sunday, not the current one
         if ((day + dts) > 31) { // The next sunday is next month !
            if (dow != 6 || hours > 0) // We finally check that we are not on the day of the change before the time of the change
               return false; // We have passed the change
            else return true;
         } else return true;
      } else return true;
    }
  }
  return false;
}

/**
 * Gets the current time adjusted for Time Zone.
 * @return The current time.
 */
unsigned long getAdjustedNTPTime() {
  unsigned long rawTime = timeClient.getEpochTime();
  return rawTime += (60 * 60 * NTP_TZ_OFFSET);
}

/**
 * Gets NTP time value adjusted for Time Zone. Useful for adjusting
 * local RTC time value.
 */
DateTime getNtpTime() {
  unsigned long adjTime = getAdjustedNTPTime();
  DateTime result = DateTime(adjTime);
  return result;
}

void onDhcpRenewTask() {

}

void onNtpUpdateTask() {

}

void sendGarageCommandPayload(uint8_t cmd) {
	if (mqttClient.connected()) {
		DynamicJsonDocument doc(200);
		doc["client_id"] = CYGARAGE_DEVICEID;
		doc["command"] = cmd;

		String jsonStr;
		size_t len = serializeJson(doc, jsonStr);
		Serial.print(F("INFO: publishing cygarage device command: "));
		Serial.println(jsonStr);
		if (!mqttClient.publish(TOPIC_GARAGE_CONTROL, jsonStr.c_str(), len)) {
			Serial.println(F("ERROR: Failed to publish message."));
		}

		doc.clear();
	}
}

void sendCyLightsCommandPayload(CyLightsControlCommand cmd) {
	if (mqttClient.connected()) {
		DynamicJsonDocument doc(200);
		doc["client_id"] = CYLIGHTS_DEVICEID;
		doc["command"] = (uint8_t)cmd;

		String jsonStr;
		size_t len = serializeJson(doc, jsonStr);
		Serial.print(F("INFO: publising cylights device command: "));
		Serial.println(jsonStr);
		if (!mqttClient.publish(TOPIC_CYLIGHTS_CONTROL, jsonStr.c_str(), len)) {
			Serial.println(F("ERROR: Failed to publish message"));
		}

		doc.clear();
	}
}

void processInput() {
	byte value = PINA;
	DeviceCommand cmd = (DeviceCommand)value;
	if (lastCmd != cmd) {
		Serial.print(F("DEBUG: Port value: "));
		Serial.println(value);
		switch (cmd) {
			case DeviceCommand::UNKNOWN:
				Serial.println(F("INFO: System status: UNKNOWN"));
				PORTC = (byte)DeviceStatus::UNKNOWN;
				break;
			case DeviceCommand::DISABLE:
				Serial.println(F("INFO: System command: DISABLE"));
				PORTC = (byte)DeviceStatus::DISABLED;
				break;
			case DeviceCommand::GARAGE_ACTIVATE:
				Serial.println(F("INFO: System command: GARAGE ACTIVATE"));
				sendGarageCommandPayload(CYGARAGE_CMD_ACTIVATE);
				break;
			case DeviceCommand::LIGHT_1_OFF:
				Serial.println(F("INFO: CyLights command: LIGHT 1 OFF"));
				sendCyLightsCommandPayload(CyLightsControlCommand::LIGHT1_OFF);
				break;
			case DeviceCommand::LIGHT_1_ON:
				Serial.println(F("INFO: CyLights command: LIGHT 1 ON"));
				sendCyLightsCommandPayload(CyLightsControlCommand::LIGHT1_ON);
				break;
		}

		lastCmd = cmd;
		cmd = DeviceCommand::UNKNOWN;
	}
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
	Serial.print(F("INFO: [MQTT] Message arrived: ["));
    Serial.print(topic);
    Serial.print(F("] "));

    // It's a lot easier to deal with if we just convert the payload
    // to a string first.
    String msg;
    for (unsigned int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }

    Serial.println(msg);

	if (PINA == (byte)DeviceCommand::DISABLE) {
		Serial.println(F("WARN: System disabled. Ignoring incoming messages."));
		return;
	}

    StaticJsonDocument<250> doc;
    DeserializationError error = deserializeJson(doc, msg);
    if (error) {
        Serial.print(F("ERROR: Failed to parse MQTT message to JSON: "));
        Serial.println(error.c_str());
        doc.clear();
        return;
    }

	if (strcmp(topic, TOPIC_GARAGE_STATUS) == 0) {
		String doorState = doc["doorState"].as<String>();
		if (doorState.length() > 0) {
			Serial.print(F("INFO: Garage state change: "));
			if (doorState == "OPEN") {
				PORTC = (uint8_t)DeviceStatus::GARAGE_OPEN;
				Serial.println(F("OPEN"));
			}
			else if (doorState == "AJAR") {
				PORTC = (uint8_t)DeviceStatus::GARAGE_AJAR;
				Serial.println(F("AJAR"));
			}
			else if (doorState == "CLOSED") {
				PORTC = (uint8_t)DeviceStatus::GARAGE_CLOSED;
				Serial.println(F("CLOSED"));
			}
		}
	}
	else if (strcmp(topic, TOPIC_CYLIGHTS_STATUS) == 0) {
		uint8_t light1State = doc["light1State"].as<uint8_t>();
		Serial.println(F("INFO: CyLights state change"));
		Serial.print(F("INFO CyLights light 1: "));
		Serial.println(light1State == HIGH ? "ON" : "OFF");
		DeviceStatus light1Status = light1State == HIGH ? 
			DeviceStatus::CL_LIGHT1_ON : DeviceStatus::CL_LIGHT1_OFF;
		PORTC = (uint8_t)light1Status;

		// TODO Get light status updates.
	}

	doc.clear();
}

bool reconnectMqttClient() {
	if (!mqttClient.connected()) {
		Serial.print(F("INFO: Attempting to establish MQTT connection to "));
		Serial.print(MQTT_BROKER);
		Serial.print(F(" on port: "));
		Serial.print(MQTT_PORT);
		Serial.println(F("... "));
		
		bool didConnect = false;
		#if defined(MQTT_USERNAME) && defined(MQTT_PASSWORD)
			didConnect = mqttClient.connect(DEVICE_NAME, MQTT_USERNAME, MQTT_PASSWORD);
		#else
			didConnect = mqttClient.connect(MQTT_BROKER);
		#endif

		if (didConnect) {
			Serial.print(F("INFO: Subscribing to channel: "));
			Serial.println(TOPIC_GARAGE_STATUS);
			mqttClient.subscribe(TOPIC_GARAGE_STATUS);
			
			Serial.print(F("INFO: Subscribing to channel: "));
			Serial.println(TOPIC_CYLIGHTS_STATUS);
			mqttClient.subscribe(TOPIC_CYLIGHTS_STATUS);

			Serial.print(F("INFO: Publishing to channel: "));
			Serial.println(TOPIC_GARAGE_CONTROL);
			Serial.print(F("INFO: Publishing to channel: "));
			Serial.println(TOPIC_CYLIGHTS_CONTROL);
		}
		else {
			String failReason = TelemetryHelper::getMqttStateDesc(mqttClient.state());
			Serial.print(F("ERROR: Failed to connect to MQTT broker: "));
			Serial.println(failReason);
			return false;
		}
	}

	return true;
}

void initSerial() {
	Serial.begin(BAUD_RATE);
	while(!Serial) {
		delay(10);
	}

	Serial.print(F("INIT: RC2014HAB Interface v"));
	Serial.print(FIRMWARE_VERSION);
	Serial.println(F(" booting..."));
}

void initInputs() {
	Serial.print(F("INIT: Initializing inputs... "));
	// Set all 8 pins on PORTA (pins 22 - 29) as inputs.
	DDRA = B00000000;
	Serial.println(F("DONE"));
}

void initOutputs() {
	Serial.print(F("INIT: Initializing outputs... "));
	// Set all 8 pins on PORTC (pins 30 - 37) as outputs and drive them low.
	DDRC = B11111111;
	PORTC = B00000000;
	Serial.println(F("DONE"));
}

void initNetwork() {
	Serial.print(F("INIT: Initializing network... "));

	bool gotAddress = false;
	uint8_t tries = 0;
	while (tries < 3) {
		if (Ethernet.begin(mac)) {
			gotAddress = true;
			break;
		}

		Serial.println(F("FAIL"));
		Serial.println(F("WARN: DHCP attempt failed."));
		tries++;
		delay(1000);
	}

	if (!gotAddress) {
		Serial.println(F("FAIL"));
		Serial.println(F("ERROR: Failed to retrieve DHCP address."));
		return;
	}

	Serial.println(F("DONE"));
	Serial.print(F("INIT: IP Address: "));
	Serial.println(Ethernet.localIP());
	delay(1000);
}

void initNtpClient() {
	Serial.print(F("INIT: Initializing NTP client... "));
	timeClient.begin();
	Serial.println(F("DONE"));
}

void initRealTimeClock() {
	Serial.print(F("INIT: Initializing RTC and synchronizing time... "));
	if (!clock.begin()) {
		Serial.println(F("FAIL"));
		Serial.println(F("ERROR: Can't find RTC!"));
	}

	#if (CLOCK_TYPE_ACTIVE == CLOCK_TYPE_DS3231)
		clock.disableAlarm(1);
		clock.disableAlarm(2);
		clock.clearAlarm(1);
		clock.clearAlarm(2);
	#endif

	if (timeClient.forceUpdate()) {
		clock.adjust(getNtpTime());
		DateTime ct = clock.now();
		Serial.println(F("DONE"));
		Serial.print(F("INFO: RTC Time from NTP: "));
		Serial.print(ct.month(), DEC);
		Serial.print(F("-"));
		Serial.print(ct.day(), DEC);
		Serial.print(F("-"));
		Serial.print(ct.year(), DEC);
		Serial.print(F(" "));
		Serial.print(ct.hour() < 10 ? "0" + String(ct.hour()) : String(ct.hour()));
		Serial.print(F(":"));
		Serial.print(ct.minute() < 10 ? "0" + String(ct.minute()) : String(ct.minute()));
		Serial.print(F(":"));
		Serial.println(ct.second() < 10 ? "0" + String(ct.second()) : String(ct.second()));

		// Adjust for DST if necessary.
		uint8_t dow = ct.dayOfTheWeek() + 1;
		if (isDST(ct.hour(), ct.day(), ct.month(), ct.year(), dow)) {
			int offset = (NTP_TZ_OFFSET - 1);
			clock.adjust(DateTime(timeClient.getEpochTime() + (60 * 60 * offset)));
		}
	}
	else {
		// If we were unable to get the time from NTP, then set the clock to the
    	// date and time that this firmware was compiled. Hopefully we can sync to
    	// *actual* time later. We only do this if the clock has not yet been
    	// initialized (first time boot or onboard battery replaced).
    	if (!clock.initialized()) {
      		clock.adjust(DateTime(F(__DATE__), F(__TIME__)));
    	}
	}
}

void initMQTT() {
	Serial.print(F("INIT: Initializing MQTT client... "));
	mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
	mqttClient.setCallback(onMqttMessage);
	Serial.println(F("DONE"));
	if (reconnectMqttClient()) {
		delay(500);
		// TODO publish sys state somewhere?
	}
}

void setup() {
	initSerial();
	initInputs();
	initOutputs();
	initNetwork();
	initNtpClient();
	initRealTimeClock();
	initMQTT();
	Serial.println(F("INIT: Boot sequence complete."));
}

void loop() {
	taskMan.execute();
	delay(1000);
	processInput();
	mqttClient.loop();
}