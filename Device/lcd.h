#ifndef DEVICE_LCD_H
#define DEVICE_LCD_H

#include "icons.h"
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

#define LCD_SIZE 240

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

class LCD {
public:
	LCD(int8_t cs, int8_t dc, int8_t rst) :
			lcd(cs, dc, rst) {}

	void clear() {
		lcd.fillScreen(ST77XX_BLACK);
	}

	void setup() {
		lcd.init(LCD_SIZE, LCD_SIZE, SPI_MODE2);
		clear();
		lcd.setTextWrap(false);
	}

	void drawLargeIcon(const uint8_t *icon, char *text1, char *text2) {
		clear();
		lcd.drawBitmap(72, 144, icon, ICON_STANDARD_SIZE, ICON_STANDARD_SIZE, ST77XX_WHITE);
		drawText(text1, 0, 24, TEXT_SMALL, TEXT_CENTER);
		drawText(text2, 0, 60, TEXT_SMALL, TEXT_CENTER);
	}

	void drawMiddleTitle(char *title) {
		drawText(title, 0, 120, TEXT_MEDIUM, TEXT_CENTER);
	}

	void drawTime(uint8_t hour, uint8_t minute) {
		char time[9];
		uint8_t hour12 = hour % 12;
		if (hour12 == 0) {
			hour12 = 12;
		}
		sprintf(time, "%d:%s%d %s", hour12, minute < 10 ? "0" : "", minute, hour < 12 ? "AM" : "PM");

		lcd.fillRect(132, 0, 108, 36, ST77XX_BLACK);
		drawText(time, 0, 24, TEXT_SMALL, TEXT_RIGHT);
	}

	void drawDayOfWeek(uint8_t dayOfWeek) {
		lcd.fillRect(0, 0, 132, 36, ST77XX_BLACK);
		drawText(DAYS_OF_WEEK[dayOfWeek], 0, 24, TEXT_SMALL, TEXT_LEFT);
	}

	void drawSensorIcon(const uint8_t *icon, uint8_t row) {
		lcd.drawBitmap(0, 36 + 48 * row, icon, ICON_SMALL_SIZE, ICON_SMALL_SIZE, ST77XX_WHITE);
	}

	void drawSensorReading(char *text, uint8_t row, bool drawDegree = false) {
		lcd.fillRect(60, 36 + 48 * row, 180, 48, ST77XX_BLACK);
		drawText(text, 60, 72 + 48 * row, TEXT_MEDIUM, TEXT_LEFT);

		if (drawDegree) {
			int16_t x1, y1;
			uint16_t width, height;
			lcd.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
			lcd.fillCircle(30 + width, 54 + 48 * row, 5, ST77XX_WHITE);
			lcd.fillCircle(30 + width, 54 + 48 * row, 3, ST77XX_BLACK);
		}
	}

	void drawWiFiDetails(char *ssid, char *ipAddress) {
		drawText(ssid, 60, 204, TEXT_SMALL, TEXT_LEFT);
		drawText(ipAddress, 60, 228, TEXT_SMALL, TEXT_LEFT);
	}

private:
	Adafruit_ST7789 lcd;
	char *DAYS_OF_WEEK[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

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
		lcd.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
		lcd.setCursor(x + align * (LCD_SIZE - width - (align == TEXT_RIGHT ? 3 : 0)) / 2, y);

		lcd.setTextColor(color);
		lcd.print(text);
	}
};

#endif