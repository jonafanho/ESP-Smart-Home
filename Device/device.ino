#include "setup.h"

const uint8_t PIN_BUTTON = D1;
const uint8_t PINS_OUTPUT[] = {D0, D2, D3, D4};
const uint8_t PINS_OUTPUT_COUNT = 4;
ESP8266WebServer server(80);
DNSServer dnsServer;
WiFiSetup wiFiSetup(server, dnsServer, "Jonathan's Smart Home Device", "index.html", "setup.html", PIN_BUTTON);

void setup() {
	for (uint8_t i = 0; i < PINS_OUTPUT_COUNT; i++) {
		pinMode(PINS_OUTPUT[i], OUTPUT);
	}

	wiFiSetup.setup();
	server.begin();
}

void loop() {
	dnsServer.processNextRequest();
	server.handleClient();
}