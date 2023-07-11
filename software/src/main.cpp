#include "main.h"
#include "mqtt.h"

#ifdef ESP8266
ESP8266WebServer server(80);
#endif
#ifdef ESP32
WebServer server(80);
#endif

OPTOLINK_CLASS optolink;

void setupLogging();

void setupHttp();

void setup() {
    setupLogging();
    println("hello! logging started");

    setupWifi(WIFI_SSID, WIFI_PASSWORD, WIFI_HOSTNAME);

    setupHttp();
    ElegantOTA.begin(&server);

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
    }
    if (!MDNS.begin("OptoProxy")) {
        println("Error setting up MDNS responder!");
    }


#ifdef ESP8266
    optolink.begin(&Serial);
#endif
#ifdef ESP32
    optolink.begin(&Serial1, OPTOLINK_SERIAL_RX, OPTOLINK_SERIAL_TX);
#endif

    if (useLogging()) {
        optolink.setLogger(getLogger());
    }
    mqttSetup();
}

OPTOLINK_CLASS *getOptolink() {
    return &optolink;
}

void loop() {
    loopWifi();
    optolink.loop();
    server.handleClient();
    mqttLoop();
}
