//
// Created by chris on 7/10/23.
//

#ifndef SOFTWARE_MQTT_H
#define SOFTWARE_MQTT_H

#include "wifi-credentials.h"

#ifdef MQTT_HOST

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#include "main.h"

#endif //ifdef MQTT_HOST

#include "optolink.h"

#define MQTT_TOPIC_BUFFER_SIZE 32
#define MQTT_VALUE_BUFFER_SIZE 16

void mqttSetup();
void mqttLoop();

class MqttDatapoint {
private:

public:
    MqttDatapoint(int address, uint8_t conversion);
    bool compareAndSend(char* newValue);
    bool send(char* newValue);
    void loop();
    bool wantsToSend();
    int address;
    uint8_t conversion;
    char lastValue[MQTT_VALUE_BUFFER_SIZE]{};
    char hexAddress[6]{};
    unsigned long sendInterval;
    unsigned long lastSend;
};

#endif //SOFTWARE_MQTT_H
