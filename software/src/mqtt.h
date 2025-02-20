//
// Created by chris on 7/10/23.
//

#ifndef SOFTWARE_MQTT_H
#define SOFTWARE_MQTT_H

#include <PubSubClient.h>

#include "configuration.h"
#include "main.h"
#include "optolink.h"

#define MQTT_TOPIC_BUFFER_SIZE 32
#define MQTT_VALUE_BUFFER_SIZE 16

void mqttSetup();
void mqttLoop();

class MqttDatapoint {
private:

public:
    MqttDatapoint(int address, uint16_t factor, uint8_t length);
    bool compareAndSend(char* newValue);
    bool send(char* newValue);
    void loop();
    bool wantsToSend() const;
    int address;
    uint16_t factor;
    uint8_t length;
    char lastValue[MQTT_VALUE_BUFFER_SIZE]{};
    char hexAddress[6]{};
    unsigned long sendInterval;
    unsigned long lastSend;
};

#endif //SOFTWARE_MQTT_H
