#include "setup.h"
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

#define PIN_BUTTON   D0
#define PIN_LCD_CS   D8
#define PIN_LCD_DC   D1
#define PIN_LCD_RST  D2
#define PIN_RELAY_CS D3

#define ACCESS_POINT_SSID "Jonathan's Smart Home Device"
#define DEFAULT_HTML      "index.html"
#define SETUP_HTML        "setup.html"

ESP8266WebServer server(80);
DNSServer dnsServer;
WiFiSetup wiFiSetup(server, dnsServer, ACCESS_POINT_SSID, DEFAULT_HTML, SETUP_HTML, PIN_BUTTON);
Adafruit_ST7789 lcd = Adafruit_ST7789(PIN_LCD_CS, PIN_LCD_DC, PIN_LCD_RST);

void drawText(char *text, uint8_t row, const uint8_t size, const uint16_t color = ST77XX_WHITE) {
	uint8_t lineSpacing;
	switch (size) {
		default:
			lineSpacing = 24;
			lcd.setFont(&FreeSans12pt7b);
			break;
		case 1:
			lineSpacing = 36;
			lcd.setFont(&FreeSans18pt7b);
			break;
		case 2:
			lineSpacing = 48;
			lcd.setFont(&FreeSans24pt7b);
			break;
	}
	lcd.setCursor(0, lineSpacing * row + lineSpacing);
	lcd.setTextColor(color);
	lcd.setTextWrap(false);
	lcd.print(text);
}

void setup() {
	pinMode(PIN_RELAY_CS, OUTPUT);

	lcd.init(240, 240, SPI_MODE2);

	wiFiSetup.setup([&](WiFiStatus wiFiStatus, char *text) {
		lcd.fillScreen(ST77XX_BLACK);
		switch (wiFiStatus) {
			case WIFI_STATUS_AP_STARTING:
				drawText("Starting Access Point", 0, 0);
				break;
			case WIFI_STATUS_AP_STARTED:
				drawText(ACCESS_POINT_SSID, 0, 0);
				break;
			case WIFI_STATUS_CONNECTING:
				drawText("Connecting to WiFi", 0, 0);
				break;
			case WIFI_STATUS_CONNECTED:
				drawText("Connected to WiFi", 0, 0);
				break;
		}
		drawText(text, 1, 0);
	});
}

void loop() {
	dnsServer.processNextRequest();
	server.handleClient();
}