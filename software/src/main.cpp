#include "main.h"
#include "mqtt.h"

XWebServer server(80);
#if defined(ESP8266)
SoftwareSerial softwareSerial(OPTOLINK_SERIAL_RX, OPTOLINK_SERIAL_TX);
#endif

void setup() {
#ifdef ESP32
    Serial1.setPins(OPTOLINK_SERIAL_RX, OPTOLINK_SERIAL_TX);
#endif

    setupLogging();
    println("hello! logging started");
    OPTOLINK_SERIAL.setTimeout(50);

    wifiMgrExpose(&server);
#ifdef WIFI_SSID
    setupWifi(WIFI_SSID, WIFI_PASSWORD, WIFI_HOSTNAME);
    server.begin();
#else
    wifiMgrConfigureEEPROM(0, 1024);
    wifiMgrPortalSetup(false, "OptoProxy-", "p0rtal123");
#endif

    setupHttp();

#ifdef WIFI_SSID
    while (WiFi.status() != WL_CONNECTED) {
        yield();
    }
    if (!MDNS.begin("OptoProxy")) {
        println("Error setting up MDNS responder!");
    }
#endif

#if defined(ESP8266)
    OPTOLINK_SERIAL.begin(4800, SWSERIAL_8E2);
#elif defined(ESP32)
    OPTOLINK_SERIAL.begin(4800, SERIAL_8E2);
#else
#error "not supported"
#endif

    mqttSetup();
}

void loop() {
#ifndef WIFI_SSID
    if (wifiMgrPortalLoop()) {
        yield();
#else
        loopWifi();
        server.handleClient();
#endif
        if (!optolinkIsLocked()) {
            loopOptolink();
            yield();
        }
        mqttLoop();
        loopHttp();
#ifndef WIFI_SSID
    }
#endif
}
