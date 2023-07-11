//
// Created by chris on 7/10/23.
//
#include "mqtt.h"

#ifdef MQTT_HOST

WiFiClient wifiClient;
PubSubClient client(wifiClient);
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
        MqttDatapoint(0x0101, 1),
        MqttDatapoint(0x0106, 1),
        MqttDatapoint(0x0105, 1),
        MqttDatapoint(0x1800, 1),
        MqttDatapoint(0x1803, 1),
        MqttDatapoint(0x010D, 1),
        MqttDatapoint(0x2006, 1),
        MqttDatapoint(0x2007, 1),
        MqttDatapoint(0x7110, 1),
        MqttDatapoint(0x7111, 1),
        MqttDatapoint(0x7103, 1),

        MqttDatapoint(0x2000, 1),
        MqttDatapoint(0x2001, 1),
        MqttDatapoint(0x0116, 1),

        MqttDatapoint(0xb000, 6),
        MqttDatapoint(0x0494, 3),
        MqttDatapoint(0x048d, 3),
        MqttDatapoint(0x0480, 3),
        MqttDatapoint(0x04a6, 3),
        MqttDatapoint(0x2005, 6),

        MqttDatapoint(0xB020, 6),
        MqttDatapoint(0x6000, 1),
        MqttDatapoint(0x600C, 1),

        MqttDatapoint(0xB420, 5),
        MqttDatapoint(0xB421, 5),
        MqttDatapoint(0xB422, 5),
        MqttDatapoint(0xB423, 5),
        MqttDatapoint(0xB424, 5),
};
uint16_t mqttDatapointpointer = 0;

char topicBuffer[MQTT_TOPIC_BUFFER_SIZE];
char valueBuffer[MQTT_VALUE_BUFFER_SIZE];

void mqttReconnect() {
    while (!client.connected()) {
	if (!WiFi.isConnected()) return;
        Serial.print("Reconnecting...");
        if (!client.connect("OptoProxy", MQTT_USER, MQTT_PASS)) {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        } else {
            Serial.print("success");
        }
    }
}

void mqttSetup() {
    client.setServer(MQTT_HOST, 1883);
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
    client.loop();

    MqttDatapoint* nextDatapoint = getNextDataPoint();
    nextDatapoint->loop();
}

MqttDatapoint::MqttDatapoint(int address, uint8_t conversion) {
    this->address = address;
    this->conversion = conversion;
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

void MqttDatapoint::compareAndSend(char* newValue) {
    if (strcmp(newValue, this->lastValue) != 0) {
        this->send(newValue);
    }
}

void MqttDatapoint::send(char* newValue) {
    strcpy(this->lastValue, newValue);
    strcpy(topicBuffer, "optoproxy/value/0x");
    strcpy(&topicBuffer[18], this->hexAddress);
    client.publish(topicBuffer, newValue, true);
}

void MqttDatapoint::loop() {
    readToBuffer(valueBuffer, MQTT_VALUE_BUFFER_SIZE, this->address, this->conversion);
    this->compareAndSend(valueBuffer);
}

#else
void mqttSetup() {}
void mqttLoop() {}
#endif