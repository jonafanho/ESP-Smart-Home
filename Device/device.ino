#include "setup.h"
#include "icons.h"
#include <Adafruit_ST7789.h>
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
Adafruit_ST7789 lcd = Adafruit_ST7789(PIN_LCD_CS, PIN_LCD_DC, PIN_LCD_RST);

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

void setup() {
	pinMode(PIN_RELAY_CS, OUTPUT);

	lcd.init(LCD_SIZE, LCD_SIZE, SPI_MODE2);
	lcd.fillScreen(ST77XX_BLACK);
	lcd.setTextWrap(false);

	wiFiSetup.setup([&](WiFiStatus wiFiStatus, char *title, char *subtitle) {
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