#include "setup.h"
#include "icons.h"
#include <Adafruit_ST7789.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

#define LCD_SIZE   240

#define PIN_BUTTON   D0
#define PIN_LCD_CS   D8
#define PIN_LCD_DC   D1
#define PIN_LCD_RST  D2
#define PIN_RELAY_CS D3

#define ACCESS_POINT_SSID "Smart Home Setup"
#define DEFAULT_HTML      "index.html"
#define SETUP_HTML        "setup.html"
#define SETTINGS_FILE     "/settings.js"

enum TextSize {
	TEXT_SMALL = 0,
	TEXT_MEDIUM = 1,
	TEXT_LARGE = 2
};
enum TextAlign {
	TEXT_LEFT = 0,
	TEXT_CENTER = 1,
	TEXT_RIGHT = 2
};

ESP8266WebServer server(80);
DNSServer dnsServer;
WiFiSetup wiFiSetup(server, dnsServer, ACCESS_POINT_SSID, DEFAULT_HTML, SETUP_HTML, PIN_BUTTON);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

Adafruit_ST7789 lcd = Adafruit_ST7789(PIN_LCD_CS, PIN_LCD_DC, PIN_LCD_RST);
StaticJsonDocument<4096> json;

void drawText(char *text, const int16_t x, const int16_t y, const TextSize size, const TextAlign align, const uint16_t color = ST77XX_WHITE) {
	switch (size) {
		default:
			lcd.setFont(&FreeSans12pt7b);
			break;
		case 1:
			lcd.setFont(&FreeSans18pt7b);
			break;
		case 2:
			lcd.setFont(&FreeSans24pt7b);
			break;
	}

	int16_t x1, y1;
	uint16_t width, height;
	lcd.getTextBounds(text, x, y, &x1, &y1, &width, &height);
	lcd.setCursor(x + align * (LCD_SIZE - width) / 2, y);

	lcd.setTextColor(color);
	lcd.print(text);
}

void writeHeader(char *text1, char *text2) {
	drawText(text1, 0, 24, TEXT_SMALL, TEXT_CENTER);
	drawText(text2, 0, 60, TEXT_SMALL, TEXT_CENTER);
}

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
	timeClient.update();

	uint8_t port = 0;
	for (JsonVariant rulesArray : json.as<JsonArray>()) {
		uint8_t rule = 0;
		for (JsonVariant ruleObject : rulesArray.as<JsonArray>()) {
			bool portOn = true;

			for (JsonPair conditionObject : ruleObject.as<JsonObject>()) {
				const char *conditionId = conditionObject.key().c_str();

				if (strcmp(conditionId, "time") == 0) {
					portOn = portOn && checkValue(conditionObject, timeClient.getHours() * 60 + timeClient.getMinutes());
				} else if (strcmp(conditionId, "dayOfWeek") == 0) {
					portOn = portOn && checkValueDiscrete(conditionObject, timeClient.getDay());
				} else if (strcmp(conditionId, "temperature") == 0) {
					portOn = portOn && checkValue(conditionObject, 25); // TODO
				} else if (strcmp(conditionId, "humidity") == 0) {
					portOn = portOn && checkValue(conditionObject, 50); // TODO
				} else if (strcmp(conditionId, "light") == 0) {
					portOn = portOn && checkValue(conditionObject, 50); // TODO
				} else if (strcmp(conditionId, "proximity") == 0) {
					portOn = portOn && checkValueDiscrete(conditionObject, 0); // TODO
				}

				if (!portOn) {
					break;
				}
			}

			if (portOn) {
				// TODO turn port on
				break;
			}
			rule++;
		}
		port++;
	}
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
	pinMode(PIN_RELAY_CS, OUTPUT);

	lcd.init(LCD_SIZE, LCD_SIZE, SPI_MODE2);
	lcd.fillScreen(ST77XX_BLACK);
	lcd.setTextWrap(false);

	wiFiSetup.setup([&]() {
		server.on("/settings", HTTP_POST, handleSettings);
	}, [&](WiFiStatus wiFiStatus, char *title, char *subtitle) {
		switch (wiFiStatus) {
			case WIFI_STATUS_AP_STARTING:
				lcd.drawBitmap(72, 144, ICON_TETHERING, ICON_STANDARD_SIZE, ICON_STANDARD_SIZE, ST77XX_WHITE);
				writeHeader("Starting Access Point", ACCESS_POINT_SSID);
				break;
			case WIFI_STATUS_AP_STARTED:
				lcd.drawBitmap(120, 192, ICON_TICK_OUTLINE, ICON_TICK_SIZE, ICON_TICK_SIZE, ST77XX_BLACK);
				lcd.drawBitmap(120, 192, ICON_TICK, ICON_TICK_SIZE, ICON_TICK_SIZE, ST77XX_GREEN);
				lcd.fillRect(0, 0, LCD_SIZE, 36, ST77XX_BLACK);
				writeHeader("Connect to WiFi:", "");
				break;
			case WIFI_STATUS_CONNECTING:
				lcd.drawBitmap(72, 144, ICON_WIFI_ON, ICON_STANDARD_SIZE, ICON_STANDARD_SIZE, ST77XX_WHITE);
				writeHeader("Connecting to WiFi", subtitle);
				break;
			case WIFI_STATUS_CONNECTED:
				lcd.drawBitmap(120, 192, ICON_TICK_OUTLINE, ICON_TICK_SIZE, ICON_TICK_SIZE, ST77XX_BLACK);
				lcd.drawBitmap(120, 192, ICON_TICK, ICON_TICK_SIZE, ICON_TICK_SIZE, ST77XX_GREEN);
				lcd.fillRect(0, 0, LCD_SIZE, 36, ST77XX_BLACK);
				writeHeader("Connected to WiFi", subtitle);
				break;
			case WIFI_STATUS_FAILED:
				lcd.fillRect(72, 144, ICON_STANDARD_SIZE, ICON_STANDARD_SIZE, ST77XX_BLACK);
				lcd.drawBitmap(72, 144, ICON_WIFI_OFF, ICON_STANDARD_SIZE, ICON_STANDARD_SIZE, ST77XX_WHITE);
				lcd.fillRect(0, 0, LCD_SIZE, 72, ST77XX_BLACK);
				writeHeader("WiFi Not Connected", "Please try again.");
				break;
		}
		lcd.fillRect(0, 84, LCD_SIZE, 54, ST77XX_BLACK);
		drawText(title, 0, 120, TEXT_MEDIUM, TEXT_CENTER);
	});
}

void loop() {
	dnsServer.processNextRequest();
	server.handleClient();
}