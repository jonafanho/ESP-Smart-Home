#ifndef SETUP_H
#define SETUP_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#define DNS_PORT 53

enum WiFiStatus {
	WIFI_STATUS_AP_STARTING = 0,
	WIFI_STATUS_AP_STARTED = 1,
	WIFI_STATUS_CONNECTING = 2,
	WIFI_STATUS_CONNECTED = 3
};

class WiFiSetup {
public:
	WiFiSetup(ESP8266WebServer &server1, DNSServer &dnsServer1, const char *apSsid, const char *defaultHtml, const char *setupHtml, const uint8_t setupPin) :
			server(server1), dnsServer(dnsServer1), AP_SSID(apSsid), DEFAULT_HTML(defaultHtml), SETUP_HTML(setupHtml), SETUP_PIN(setupPin) {}

	void setup(void (*callback)(WiFiStatus, char *)) {
		SPIFFS.begin();
		pinMode(SETUP_PIN, OUTPUT);

		WiFi.persistent(false);
		WiFi.disconnect(true);
		if (digitalRead(SETUP_PIN)) {
			(*callback)(WIFI_STATUS_AP_STARTING, "");
			WiFi.mode(WIFI_AP);
			const IPAddress apIp(192, 168, 0, 1);
			WiFi.softAPConfig(apIp, apIp, IPAddress(255, 255, 255, 0));
			WiFi.softAP(AP_SSID);
			dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
			dnsServer.start(DNS_PORT, "*", apIp);
			server.onNotFound([&]() { onRequest(SETUP_HTML); });
			server.begin();
			(*callback)(WIFI_STATUS_AP_STARTED, const_cast<char *> (apIp.toString().c_str()));
		} else {
			connectToWifi(callback);
		}
	}

private:
	ESP8266WebServer &server;
	DNSServer &dnsServer;
	const char *AP_SSID, *DEFAULT_HTML, *SETUP_HTML;
	const uint8_t SETUP_PIN;

	bool connectToWifi(void (*callback)(WiFiStatus, char *)) {
		(*callback)(WIFI_STATUS_CONNECTING, "");
		File wifiFile = SPIFFS.open("/wifi.txt", "r");
		if (wifiFile) {
			char ssid[64], password[64];
			ssid[wifiFile.readBytesUntil('\n', ssid, sizeof(ssid) - 1)] = 0;
			password[wifiFile.readBytesUntil('\n', password, sizeof(password) - 1)] = 0;
			wifiFile.close();

			WiFi.mode(WIFI_STA);
			WiFi.begin(ssid, password);
			for (uint8_t i = 0; i < 10; i++) {
				if (WiFi.status() != WL_CONNECTED) {
					delay(1000);
				} else {
					server.onNotFound([&]() { onRequest(DEFAULT_HTML); });
					server.begin();
					(*callback)(WIFI_STATUS_CONNECTED, ssid);
					return true;
				}
			}
		} else {
			return false;
		}
	}

	void onRequest(const char *defaultPath) {
		char path[64];
		sprintf(path, "%s", server.uri().c_str());
		if (!SPIFFS.exists(path)) {
			sprintf(path, "/%s", defaultPath);
		}
		File file = SPIFFS.open(path, "r");
		server.streamFile(file, getContentType(path));
		file.close();
	}

	static String getContentType(String filename) {
		if (filename.endsWith(".html")) return "text/html";
		else if (filename.endsWith(".css")) return "text/css";
		else if (filename.endsWith(".js")) return "application/javascript";
		else if (filename.endsWith(".ico")) return "image/x-icon";
		else return "text/plain";
	}
};

#endif