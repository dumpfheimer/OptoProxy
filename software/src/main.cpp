#include "main.h"
#include "mqtt.h"
#include "VitoWiFi.h"

XWebServer server(80);

#ifdef ESP8266
OPTOLINK_CLASS optolink(&Serial);
#endif
#ifdef ESP32
OPTOLINK_CLASS optolink(&OPTOLINK_SERIAL);
#endif

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

    optolink.begin();

    mqttSetup();
}

OPTOLINK_CLASS *getOptolink() {
    return &optolink;
}
Stream *getOptolinkSerial() {
    return &OPTOLINK_SERIAL;
}

void loop() {
#ifndef WIFI_SSID
    if (wifiMgrPortalLoop()) {
#else
        loopWifi();
#endif
        optolink.loop();
        server.handleClient();
        mqttLoop();
        loopHttp();
#ifndef WIFI_SSID
    }
#endif
}
