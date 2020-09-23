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
		writeHeader(text1, text2);
	}

	void drawMiddleTitle(char *title) {
		lcd.fillRect(0, 84, LCD_SIZE, 54, ST77XX_BLACK);
		drawText(title, 0, 120, TEXT_MEDIUM, TEXT_CENTER);
	}

private:
	Adafruit_ST7789 lcd;

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
};

#endif