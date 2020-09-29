#ifndef SETUP_H
#define SETUP_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#define DNS_PORT  53
#define WIFI_FILE "/wifi.txt"

enum WiFiStatus {
	WIFI_STATUS_AP = 0,
	WIFI_STATUS_CONNECTING = 1,
	WIFI_STATUS_CONNECTED = 2,
	WIFI_STATUS_FAILED = 3
};

class WiFiSetup {
public:
	WiFiSetup(ESP8266WebServer &server1, DNSServer &dnsServer1, const char *apSsid, const char *defaultHtml, const char *setupHtml, const uint8_t setupPin) :
			server(server1), dnsServer(dnsServer1), AP_SSID(apSsid), DEFAULT_HTML(defaultHtml), SETUP_HTML(setupHtml), SETUP_PIN(setupPin) {}

	void setup(void (*bindServer)(), void (*callback)(WiFiStatus, char *)) {
		SPIFFS.begin();
		pinMode(SETUP_PIN, OUTPUT);

		WiFi.persistent(false);
		WiFi.disconnect(true);

		if (digitalRead(SETUP_PIN)) {
			startAccessPoint(callback);
		} else {
			connectToWifi(bindServer, callback);
		}
	}

private:
	ESP8266WebServer &server;
	DNSServer &dnsServer;
	const char *AP_SSID, *DEFAULT_HTML, *SETUP_HTML;
	const uint8_t SETUP_PIN;

	void startAccessPoint(void (*callback)(WiFiStatus, char *)) {
		WiFi.mode(WIFI_AP);
		const IPAddress apIp(192, 168, 0, 1);
		WiFi.softAPConfig(apIp, apIp, IPAddress(255, 255, 255, 0));
		WiFi.softAP(AP_SSID);
		dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
		dnsServer.start(DNS_PORT, "*", apIp);

		server.onNotFound([&]() { onRequest(SETUP_HTML); });
		server.on("/wifi", HTTP_POST, [&]() {
			File wifiFile = SPIFFS.open(WIFI_FILE, "w");
			if (wifiFile) {
				wifiFile.println(server.arg("ssid"));
				wifiFile.println(server.arg("password"));
				wifiFile.close();
				server.send(200, "text/html", "{\"status\":\"success\"}");
			}
		});
		server.begin();

		delay(10);
		(*callback)(WIFI_STATUS_AP, const_cast<char *> (apIp.toString().c_str()));
		delay(10);

		while (true) {
			dnsServer.processNextRequest();
			server.handleClient();
		}
	}

	bool connectToWifi(void (*bindServer)(), void (*callback)(WiFiStatus, char *)) {
		File wifiFile = SPIFFS.open(WIFI_FILE, "r");
		if (wifiFile) {
			char ssid[64], password[64];
			const uint8_t ssidLength = wifiFile.readBytesUntil('\n', ssid, sizeof(ssid) - 1);
			if (ssid[ssidLength - 1] == '\r') {
				ssid[ssidLength - 1] = 0;
			} else {
				ssid[ssidLength] = 0;
			}
			const uint8_t passwordLength = wifiFile.readBytesUntil('\n', password, sizeof(password) - 1);
			if (password[passwordLength - 1] == '\r') {
				password[passwordLength - 1] = 0;
			} else {
				password[passwordLength] = 0;
			}
			wifiFile.close();

			delay(10);
			(*callback)(WIFI_STATUS_CONNECTING, ssid);
			delay(10);

			WiFi.mode(WIFI_STA);
			WiFi.begin(ssid, password);
			for (uint8_t i = 0; i < 10; i++) {
				if (WiFi.status() != WL_CONNECTED) {
					delay(2000);
				} else {
					server.onNotFound([&]() { onRequest(DEFAULT_HTML); });
					(*bindServer)();
					server.begin();

					delay(10);
					(*callback)(WIFI_STATUS_CONNECTED, const_cast<char *> (WiFi.localIP().toString().c_str()));
					delay(10);

					return true;
				}
			}
		}

		delay(10);
		(*callback)(WIFI_STATUS_FAILED, "");
		delay(10);

		return false;
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