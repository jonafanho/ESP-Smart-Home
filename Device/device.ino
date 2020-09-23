#include "setup.h"
#include "lcd.h"
#include "icons.h"
#include <ArduinoJson.h>
#include <DHTesp.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define PIN_BUTTON   D0
#define PIN_MOTION   D1
#define PIN_RELAY_CS D2
#define PIN_LCD_DC   D3
#define PIN_LCD_RST  D4
#define PIN_DHT11    D6
#define PIN_LCD_CS   D8
#define PIN_LIGHT    A0

#define ACCESS_POINT_SSID "Smart Home Setup"
#define DEFAULT_HTML      "index.html"
#define SETUP_HTML        "setup.html"
#define SETTINGS_FILE     "/settings.js"

#define POLL_SENSORS_MILLIS 5000
#define MOTION_COOLDOWN     10000

ESP8266WebServer server(80);
DNSServer dnsServer;
WiFiSetup wiFiSetup(server, dnsServer, ACCESS_POINT_SSID, DEFAULT_HTML, SETUP_HTML, PIN_BUTTON);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

LCD lcd = LCD(PIN_LCD_CS, PIN_LCD_DC, PIN_LCD_RST);
DHTesp dht;
StaticJsonDocument<4096> json;

unsigned long oldMillis = 0;
unsigned long motionOffMillis = 0;
bool sensorMotion = false;
int8_t sensorTemperature = 0;
int8_t sensorHumidity = 0;
uint8_t sensorLight = 0;
uint8_t sensorHour = 0;
uint8_t sensorMinute = 0;
uint8_t sensorDayOfWeek = 0;
char *ipAddress;

bool checkValue(const JsonPair conditionObject, const int16_t actual) {
	uint8_t comparison = conditionObject.value()["comparison"].as<uint8_t>();
	int16_t min = conditionObject.value()["min"].as<int16_t>();
	int16_t max = conditionObject.value()["max"].as<int16_t>();
	switch (comparison) {
		case 0:
			return actual <= max;
		case 1:
			return actual >= min;
		case 2:
			return actual >= min && actual <= max;
		case 3:
			return actual <= min || actual >= max;
		default:
			return false;
	}
}

bool checkValueDiscrete(const JsonPair conditionObject, const int8_t actual) {
	for (JsonVariant item : conditionObject.value()["values"].as<JsonArray>()) {
		if (item.as<uint8_t>() == actual) {
			return actual >= 0;
		}
	}
	return actual < 0;
}

void updateStatus() {
	uint8_t portResult = 0;
	uint8_t port = 0;
	for (JsonVariant rulesArray : json.as<JsonArray>()) {
		uint8_t rule = 0;
		for (JsonVariant ruleObject : rulesArray.as<JsonArray>()) {
			bool portOn = true;

			for (JsonPair conditionObject : ruleObject.as<JsonObject>()) {
				const char *conditionId = conditionObject.key().c_str();

				if (strcmp(conditionId, "time") == 0) {
					portOn = portOn && checkValue(conditionObject, sensorHour * 60 + sensorMinute);
				} else if (strcmp(conditionId, "dayOfWeek") == 0) {
					portOn = portOn && checkValueDiscrete(conditionObject, sensorDayOfWeek);
				} else if (strcmp(conditionId, "temperature") == 0) {
					portOn = portOn && checkValue(conditionObject, sensorTemperature);
				} else if (strcmp(conditionId, "humidity") == 0) {
					portOn = portOn && checkValue(conditionObject, sensorHumidity);
				} else if (strcmp(conditionId, "light") == 0) {
					portOn = portOn && checkValue(conditionObject, sensorLight);
				} else if (strcmp(conditionId, "proximity") == 0) {
					portOn = portOn && checkValueDiscrete(conditionObject, sensorMotion ? 0 : -1);
				}

				if (!portOn) {
					break;
				}
			}

			if (portOn) {
				portResult |= (1 << port);
				break;
			}
			rule++;
		}
		port++;
	}

	digitalWrite(PIN_RELAY_CS, LOW);
	SPI.transfer(portResult);
	digitalWrite(PIN_RELAY_CS, HIGH);
}

void handleSettings() {
	timeClient.setTimeOffset(server.arg("timezone").toInt() * -60);

	File settingsFile = SPIFFS.open(SETTINGS_FILE, "w");
	if (settingsFile) {
		settingsFile.print("const STORED_SETTINGS=");
		settingsFile.print(server.arg("plain"));
		settingsFile.close();

		if (!deserializeJson(json, server.arg("plain"))) {
			server.send(200, "text/html", "{\"status\":\"success\"}");
			updateStatus();
		}
	}
}

void setup() {
	lcd.setup();
	pinMode(PIN_MOTION, INPUT);
	pinMode(PIN_RELAY_CS, OUTPUT);
	dht.setup(PIN_DHT11, DHTesp::DHT11);

	wiFiSetup.setup([&]() {
		server.on("/settings", HTTP_POST, handleSettings);
	}, [&](WiFiStatus wiFiStatus, char *subtitle) {
		switch (wiFiStatus) {
			case WIFI_STATUS_AP:
				lcd.drawLargeIcon(ICON_TETHERING, "Connect to WiFi:", ACCESS_POINT_SSID);
				lcd.drawMiddleTitle(subtitle);
				break;
			case WIFI_STATUS_CONNECTING:
				lcd.drawLargeIcon(ICON_WIFI_ON, "Connecting to WiFi", subtitle);
				break;
			case WIFI_STATUS_CONNECTED:
				ipAddress = subtitle;
				break;
			case WIFI_STATUS_FAILED:
				lcd.drawLargeIcon(ICON_WIFI_OFF, "WiFi Not Connected", "Please try again.");
				break;
		}
	});
}

void loop() {
	timeClient.update();
	uint8_t tempHour = timeClient.getHours();
	uint8_t tempMinute = timeClient.getMinutes();
	uint8_t tempDayOfWeek = timeClient.getDay();

	TempAndHumidity tempAndHumidity = dht.getTempAndHumidity();
	int8_t tempTemperature = round(tempAndHumidity.temperature);
	int8_t tempHumidity = round(tempAndHumidity.humidity);

	uint8_t tempLight = (uint8_t)(analogRead(PIN_LIGHT) / 10.24 + 0.5);

	sensorHour = tempHour;
	sensorMinute = tempMinute;
	sensorDayOfWeek = tempDayOfWeek;
	if (tempHumidity >= 0) {
		sensorTemperature = tempTemperature;
		sensorHumidity = tempHumidity;
	}
	sensorLight = tempLight;

	updateStatus();

	while (millis() - oldMillis < POLL_SENSORS_MILLIS) {
		dnsServer.processNextRequest();
		server.handleClient();

		if (digitalRead(PIN_MOTION)) {
			if (!sensorMotion) {
				sensorMotion = true;
				motionOffMillis = 0;
				updateStatus();
				oldMillis = millis();
			}
		} else {
			if (motionOffMillis == 0) {
				motionOffMillis = millis();
			} else if (millis() - motionOffMillis > MOTION_COOLDOWN && sensorMotion) {
				sensorMotion = false;
				updateStatus();
				oldMillis = millis();
			}
		}
	}
	oldMillis = millis();
}