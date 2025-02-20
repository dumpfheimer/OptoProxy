//
// Created by chris on 7/10/23.
//

#ifndef SOFTWARE_OPTOLINK_H
#define SOFTWARE_OPTOLINK_H

#include <Arduino.h>
#include "Datapoint/Datapoint.h"
#include "main.h"

bool readToBuffer(char* buffer, VitoWiFi::Datapoint datapoint);
bool readToBuffer(char* buffer, uint16_t addr, uint8_t len, VitoWiFi::Converter *converter);
String readToString(VitoWiFi::Datapoint datapoint);
String readToString(uint16_t addr, uint8_t len, VitoWiFi::Converter *converter);
bool writeFromString(VitoWiFi::Datapoint datapoint, String value);
bool writeFromString(VitoWiFi::Datapoint datapoint, String value, char* readTo);

#endif //SOFTWARE_OPTOLINK_H
