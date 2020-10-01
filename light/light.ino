#include <Servo.h>

#define PIN_SERVO     PB8
#define PIN_MOTION    PB9
#define PIN_LED_2     PC15
#define PIN_LED_1     PB7
#define PIN_LED_READY PC14

#define TWO_MINUTES 120000
#define ONE_MINUTE   60000
#define FLASHING     10000

Servo servo;

void drawLights(bool two, bool one, bool ready) {
	digitalWrite(PIN_LED_2, two);
	digitalWrite(PIN_LED_1, one);
	digitalWrite(PIN_LED_READY, ready);
}

void setup() {
	delay(500);
	servo.attach(PIN_SERVO);
	delay(500);
	servo.write(90);
	delay(500);

	pinMode(PIN_MOTION, INPUT);
	pinMode(PIN_LED_2, OUTPUT);
	pinMode(PIN_LED_1, OUTPUT);
	pinMode(PIN_LED_READY, OUTPUT);
	drawLights(0, 0, 1);
}

unsigned long motionOffMillis = 0;
bool sensorMotion = true;

void loop() {
	if (digitalRead(PIN_MOTION)) {
		drawLights(1, 0, 0);
		motionOffMillis = 0;

		if (!sensorMotion) {
			sensorMotion = true;

			delay(500);
			servo.write(40);
			delay(500);
			servo.write(90);
			delay(500);
		}
	} else {
		if (motionOffMillis == 0) {
			motionOffMillis = millis();
		} else if (sensorMotion) {
			if (millis() - motionOffMillis > TWO_MINUTES) {
				drawLights(0, 0, 1);
				sensorMotion = false;

				delay(500);
				servo.write(140);
				delay(500);
				servo.write(90);
				delay(500);
			} else if (millis() - motionOffMillis > ONE_MINUTE) {
				bool on = millis() - motionOffMillis < TWO_MINUTES - FLASHING || (millis() % 1000) > 500;
				drawLights(0, on, 0);
			} else {
				bool on = millis() - motionOffMillis < ONE_MINUTE - FLASHING || (millis() % 1000) > 500;
				drawLights(on, 0, 0);
			}
		}
	}
}