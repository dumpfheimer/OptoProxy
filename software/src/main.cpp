#include "main.h"
#include "mqtt.h"

XWebServer server(80);
SoftwareSerial softwareSerial(D4, D5);

void setup() {
#ifdef ESP32
    Serial1.setPins(OPTOLINK_SERIAL_RX, OPTOLINK_SERIAL_TX);
#endif

    setupLogging();
    println("hello! logging started");

    wifiMgrExpose(&server);
#ifdef WIFI_SSID
    setupWifi(WIFI_SSID, WIFI_PASSWORD, WIFI_HOSTNAME);
#else
    wifiMgrPortalSetup(false);
#endif

    setupHttp();
    ElegantOTA.begin(&server);

#ifdef WIFI_SSID
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
    }
    if (!MDNS.begin("OptoProxy")) {
        println("Error setting up MDNS responder!");
    }
#endif

    OPTOLINK_SERIAL.begin(4800, SWSERIAL_8E2);

    mqttSetup();
}

void loop() {
#ifndef WIFI_SSID
    if (wifiMgrPortalLoop()) {
#else
        loopWifi();
#endif
        loopOptolink();
        server.handleClient();
        mqttLoop();
        loopHttp();
#ifndef WIFI_SSID
    }
#endif
}
