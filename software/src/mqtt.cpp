//
// Created by chris on 7/10/23.
//
#include "mqtt.h"

WiFiClient wifiClient;
PubSubClient client(wifiClient);
unsigned long lastConnect = 0 - 5000;

/*
    0 raw ?
    1 temp 2_10_UL
    2 temps 1_1_US
    3 stat 1_1_B
    4 count 4_1_UL
    5 counts 2_1_UL
    6 mode 1_1_US
    7 hours 4_360_F
    8 cop 1_10_F
    */
MqttDatapoint mqttDatapoints[] = {
        MqttDatapoint(0x0101, 10, 2),
        MqttDatapoint(0x0106, 10, 2),
        MqttDatapoint(0x0105, 10, 2),
        MqttDatapoint(0x0847, 1, 1),
        MqttDatapoint(0x1800, 10, 2),
        MqttDatapoint(0x1803, 10, 2),
        MqttDatapoint(0x010D, 10, 2),
        MqttDatapoint(0x2006, 10, 2),
        MqttDatapoint(0x2007, 10, 2),
        MqttDatapoint(0x7110, 10, 2),
        MqttDatapoint(0x7111, 10, 2),
        MqttDatapoint(0x7103, 10, 2),

        MqttDatapoint(0x2000, 10, 2),
        MqttDatapoint(0x2001, 10, 2),
        MqttDatapoint(0x0116, 10, 2),

        MqttDatapoint(0xb000, 1, 1),
        MqttDatapoint(0x0494, 1, 1),
        MqttDatapoint(0x048d, 1, 1),
        MqttDatapoint(0x0480, 1, 1),
        MqttDatapoint(0x04a6, 1, 1),
        MqttDatapoint(0x2005, 1, 1),

        MqttDatapoint(0xB020, 1, 1),
        MqttDatapoint(0x6000, 10, 2),
        MqttDatapoint(0x600C, 10, 2),

        MqttDatapoint(0xB420, 1, 2),
        MqttDatapoint(0xB421, 1, 2),
        MqttDatapoint(0xB422, 1, 2),
        MqttDatapoint(0xB423, 1, 2),
        MqttDatapoint(0xB424, 1, 2),
};
uint16_t mqttDatapointpointer = 0;

char *topicBuffer = new char[MQTT_TOPIC_BUFFER_SIZE];
char *valueBuffer = new char[MQTT_VALUE_BUFFER_SIZE];
char *receiveBuffer = new char[MQTT_VALUE_BUFFER_SIZE];

void mqttReconnect() {
    if (!client.connected()) {
        if (!WiFi.isConnected()) return;
        if (millis() - lastConnect > 5000) {
            Serial.print("Reconnecting...");
#ifndef MQTT_USER
            const char* MQTT_USER = wifiMgrGetConfig("MQTT_USER");
            if (MQTT_USER == nullptr) return;
#endif
#ifndef MQTT_PASS
            const char* MQTT_PASS = wifiMgrGetConfig("MQTT_PASS");
            if (MQTT_PASS == nullptr) return;
#endif
            if (!client.connect("OptoProxy", MQTT_USER, MQTT_PASS)) {
                Serial.print("failed, rc=");
                Serial.print(client.state());
                Serial.println(" retrying in 5 seconds");
            } else {
                Serial.print("success");
                client.subscribe("optoproxy/request");
            }
            lastConnect = millis();
        }
    }
}

void onMqttMessage(char *topic, byte *payload, unsigned int length) {
    if (length > 31) return;
    int i;
    for (i = 0; i < length; i++) receiveBuffer[i] = (char) payload[i];
    receiveBuffer[i] = '\0';
    char *part = strtok(receiveBuffer, ":");
    i = 0;

    if (strcmp(topic, "optoproxy/request") == 0) {
        uint16_t addr = 0;
        uint8_t len = 0;
        uint16_t factor = 1;

        while (part != nullptr) {
            if (i == 0) {
                // addr
                addr = strtoul(part, nullptr, 16);
            } else if (i == 1) {
                // conv
                if (strcmp(part, "raw") == 0) {
                    len = 4;
                } else if (strcmp(part, "temp") == 0) {
                    factor = 10;
                    len = 2;
                } else if (strcmp(part, "temps") == 0 || strcmp(part, "percent") == 0) {
                    len = 2;
                } else if (strcmp(part, "stat") == 0) {
                    len = 1;
                } else if (strcmp(part, "count") == 0) {
                    len = 4;
                } else if (strcmp(part, "counts") == 0) {
                    len = 2;
                } else if (strcmp(part, "mode") == 0) {
                    len = 1;
                } else if (strcmp(part, "hours") == 0) {
                    factor = 3600;
                    len = 4;
                } else if (strcmp(part, "cop") == 0) {
                    factor = 10;
                    len = 1;
                } else {
                    len = 1;
                }
            } else if (i == 2) {
                // len
                len = strtoul(part, nullptr, 10);
            }
            part = strtok(nullptr, ":"); // Extract the next token
            i++;
        }
        float f;
        readToBuffer(receiveBuffer, &f, addr, len, factor);

        strcpy(topicBuffer, "optoproxy/value/0x");
        sprintf(&topicBuffer[18], "%04X", addr);
        client.publish(topicBuffer, receiveBuffer, false);
    }
}

void mqttSetup() {
#ifdef MQTT_HOST
    client.setServer(MQTT_HOST, 1883);
#else
    wifiMgrPortalAddConfigEntry("MQTT Host", "MQTT_HOST", PortalConfigEntryType::STRING, false, true);
    wifiMgrPortalAddConfigEntry("MQTT Username", "MQTT_USER", PortalConfigEntryType::STRING, false, true);
    wifiMgrPortalAddConfigEntry("MQTT Password", "MQTT_PASS", PortalConfigEntryType::STRING, true, true);

    const char* MQTT_HOST = wifiMgrGetConfig("MQTT_HOST");
    if (MQTT_HOST == nullptr) return;
    client.setServer(MQTT_HOST, 1883);
#endif
    client.setBufferSize(350);
    client.setCallback(onMqttMessage);
}

MqttDatapoint* getNextDataPoint() {
    MqttDatapoint* ret = &mqttDatapoints[mqttDatapointpointer];
    mqttDatapointpointer++;
    if (mqttDatapointpointer >= (sizeof(mqttDatapoints) / sizeof(mqttDatapoints[0]))) {
        mqttDatapointpointer = 0;
    }
    return ret;
}

void mqttLoop() {
    if (!client.connected()) {
        mqttReconnect();
    }
    if (client.connected()) {
        client.loop();

        MqttDatapoint *nextDatapoint = getNextDataPoint();
        nextDatapoint->loop();
    }
}

MqttDatapoint::MqttDatapoint(int address, uint16_t factor, uint8_t length) {
    this->address = address;
    this->factor = factor;
    this->length = length;
    this->lastValue[0] = 0;
    this->sendInterval = 30000;
    this->lastSend = 0 - this->sendInterval;
    int shift = 0;
    this->hexAddress[0] = '0';
    this->hexAddress[1] = '0';
    this->hexAddress[2] = '0';

    if (address <= 0x000F) {
        shift = 3;
    } else if (address <= 0x00FF) {
        shift = 2;
    } else if (address <= 0x0FFF) {
        shift = 1;
    }

    ltoa(address, this->hexAddress + shift, 16);
}

bool MqttDatapoint::compareAndSend(char* newValue) {
    if (strcmp(newValue, this->lastValue) != 0) {
        return this->send(newValue);
    }
    return false;
}
bool mqttLock = false;
bool MqttDatapoint::send(char* newValue) {
    while (mqttLock) delay(1);
    mqttLock = true;
    strcpy(topicBuffer, "optoproxy/value/0x");
    strcpy(&topicBuffer[18], this->hexAddress);
    bool ret = client.publish(topicBuffer, newValue, true);
    client.loop();
    if (ret) {
        this->lastSend = millis();
        strcpy(this->lastValue, newValue);
    }
    mqttLock = false;
    return ret;
}

void MqttDatapoint::loop() {
    float f;
    /*if (readToBuffer(valueBuffer, &f, this->address, this->length, this->converter)) {
        this->compareAndSend(valueBuffer) || (wantsToSend() && send(valueBuffer));
    }*/
}

bool MqttDatapoint::wantsToSend() const {
    return millis() - this->lastSend >= this->sendInterval;
}