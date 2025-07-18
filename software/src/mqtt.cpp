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
        MqttDatapoint(0x0101, 10, 2, true, false),
        MqttDatapoint(0x0106, 10, 2, false, false),
        MqttDatapoint(0x0105, 10, 2, false, false),
        MqttDatapoint(0x0847, 1, 1, false, false),
        MqttDatapoint(0x1800, 10, 2, false, false),
        MqttDatapoint(0x1803, 10, 2, false, false),
        MqttDatapoint(0x010D, 10, 2, false, false),
        MqttDatapoint(0x2006, 10, 2, true, false),
        MqttDatapoint(0x2007, 10, 2, true, false),
        MqttDatapoint(0x7110, 10, 2, true, false),
        MqttDatapoint(0x7111, 10, 2, true, false),
        MqttDatapoint(0x7103, 10, 2, false, false),

        MqttDatapoint(0x2000, 10, 2, false, false),
        MqttDatapoint(0x2001, 10, 2, false, false),
        MqttDatapoint(0x0116, 10, 2, false, false),

        MqttDatapoint(0xb000, 1, 1, false, false),
        MqttDatapoint(0x0494, 1, 1, false, false),
        MqttDatapoint(0x048d, 1, 1, false, false),
        MqttDatapoint(0x0480, 1, 1, false, false),
        MqttDatapoint(0x04a6, 1, 1, false, false),
        MqttDatapoint(0x2005, 1, 1, false, false),

        MqttDatapoint(0xB020, 1, 1, false, false),
        MqttDatapoint(0x6000, 10, 2, false, false),
        MqttDatapoint(0x600C, 10, 2, false, false),

        MqttDatapoint(0xB420, 1, 2, false, false),
        MqttDatapoint(0xB421, 1, 2, false, false),
        MqttDatapoint(0xB422, 1, 2, false, false),
        MqttDatapoint(0xB423, 1, 2, false, false),
        MqttDatapoint(0xB424, 1, 2, false, false),
};
uint16_t mqttDatapointpointer = 0;

void mqttReconnect() {
    if (!client.connected()) {
        if (!WiFi.isConnected()) return;
        if (millis() - lastConnect > 5000) {
#ifndef MQTT_USER
            const char* MQTT_USER = wifiMgrGetConfig("MQTT_USER");
            if (MQTT_USER == nullptr) return;
#endif
#ifndef MQTT_PASS
            const char* MQTT_PASS = wifiMgrGetConfig("MQTT_PASS");
            if (MQTT_PASS == nullptr) return;
#endif
            if (!client.connect(WiFi.getHostname(), MQTT_USER, MQTT_PASS)) {
            } else {
                client.subscribe("optoproxy/request");
            }
            lastConnect = millis();
        }
    }
}

void onMqttMessage(char *topic, byte *payload, unsigned int length) {
    if (length > MQTT_VALUE_BUFFER_SIZE - 2) return;
    unsigned int i;

    if (strcmp(topic, "optoproxy/request") == 0) {
        char *receiveBuffer = (char*) malloc(sizeof(char) * MQTT_VALUE_BUFFER_SIZE);
        if (receiveBuffer == nullptr) return;

        for (i = 0; i < length && i < MQTT_VALUE_BUFFER_SIZE - 1; i++) receiveBuffer[i] = (char) payload[i];
        receiveBuffer[i] = '\0';
        char *part = strtok(receiveBuffer, ":");
        i = 0;

        auto *config = (DatapointConfig *)(malloc(sizeof(DatapointConfig)));
        if (config == nullptr) return;
        config->sign = false;
        config->hex = false;
        config->factor = 1;
        config->addr = 0;
        config->len = 0;

        while (part != nullptr) {
            if (i == 0) {
                // addr
                config->addr = strtoul(part, nullptr, 16);
            } else if (i == 1) {
                // conv
                if (strcmp(part, "raw") == 0 || strcmp(part, "count") == 0) {
                    config->len = 4;
                } else if (strcmp(part, "temp") == 0) {
                    config->factor = 10;
                    config->len = 2;
                } else if (strcmp(part, "temps") == 0 || strcmp(part, "percent") == 0 || strcmp(part, "counts") == 0) {
                    config->len = 2;
                } else if (strcmp(part, "hours") == 0) {
                    config->factor = 3600;
                    config->len = 4;
                } else if (strcmp(part, "cop") == 0) {
                    config->factor = 10;
                    config->len = 1;
                //} else if (strcmp(part, "stat") == 0 || strcmp(part, "mode") == 0) {
                //    config->len = 1;
                } else {
                    config->len = 1;
                }
            } else if (i == 2) {
                // len
                config->len = strtoul(part, nullptr, 10);
            } else if (i == 3) {
                // len
                if (strcmp(part, "yes") == 0 || strcmp(part, "on") == 0) config->sign = true;
                else config->sign = false;
                if (strcmp(part, "hex") == 0) config->hex = true;
            }
            part = strtok(nullptr, ":"); // Extract the next token
            i++;
        }

        if (config->len > 0) { // Only proceed if we have a valid length
            readToBuffer(receiveBuffer, MQTT_VALUE_BUFFER_SIZE, config);

            char* tmpBuffer = (char*) malloc(sizeof(char) * (18 + 4 + 1));
            if (tmpBuffer != nullptr) {
                strncpy(tmpBuffer, "optoproxy/value/0x", 19);
                snprintf(&tmpBuffer[18], 5, "%04X", config->addr);
                if (client.publish(tmpBuffer, receiveBuffer, false)) {
                    client.loop();
                    wifiClient.flush();
                }
                free(tmpBuffer);
            }
        }
        free(config);
        free(receiveBuffer);
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

        if (!optolinkIsLocked()) {
            MqttDatapoint *nextDatapoint = getNextDataPoint();
            nextDatapoint->loop();
        }
    }
}

MqttDatapoint::MqttDatapoint(int address, uint16_t factor, uint8_t length, bool sign, bool printHex) {
    this->address = address;
    this->factor = factor;
    this->length = length;
    this->sign = sign;
    this->lastValue[0] = 0;
    this->sendInterval = 30000;
    this->lastSend = 0 - this->sendInterval;
    int shift = 0;
    this->hexAddress[0] = '0';
    this->hexAddress[1] = '0';
    this->hexAddress[2] = '0';
    this->hexAddress[3] = '\0';
    this->hexAddress[4] = '\0';
    this->hexAddress[5] = '\0';
    this->printHex = printHex;

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

bool MqttDatapoint::send(char* newValue) {
    if (newValue == nullptr) return false;

    char *topicBuffer = (char*) malloc(sizeof(char) * MQTT_TOPIC_BUFFER_SIZE);
    if (topicBuffer == nullptr) return false;

    strncpy(topicBuffer, "optoproxy/value/0x", MQTT_TOPIC_BUFFER_SIZE);
    strncpy(&topicBuffer[18], this->hexAddress, MQTT_TOPIC_BUFFER_SIZE - 18);
    bool ret = client.publish(topicBuffer, newValue, true);
    if (ret) {
        client.loop();
        wifiClient.flush();
        this->lastSend = millis();
        strncpy(this->lastValue, newValue, MQTT_VALUE_BUFFER_SIZE);
    }
    free(topicBuffer);
    return ret;
}

void MqttDatapoint::loop() {
    auto *config = (DatapointConfig *)(malloc(sizeof(DatapointConfig)));
    if (config != nullptr) {
        config->addr = this->address;
        config->len = this->length;
        config->factor = this->factor;
        config->sign = this->sign;
        config->hex = this->printHex;

        char *valueBuffer = (char*) malloc(sizeof(char) * MQTT_VALUE_BUFFER_SIZE);

        if (valueBuffer == nullptr) {
            free(config);
            return;
        }
        if (readToBuffer(valueBuffer, MQTT_VALUE_BUFFER_SIZE, config)) {
            this->compareAndSend(valueBuffer) || (wantsToSend() && send(valueBuffer));
        }
        free(valueBuffer);
        free(config);
    }
}

bool MqttDatapoint::wantsToSend() const {
    return millis() - this->lastSend >= this->sendInterval;
}