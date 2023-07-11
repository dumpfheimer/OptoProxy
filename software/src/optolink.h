//
// Created by chris on 7/10/23.
//

#ifndef SOFTWARE_OPTOLINK_H
#define SOFTWARE_OPTOLINK_H

#include <Arduino.h>
#include <DPValue.hpp>
#include <Datapoint.hpp>
#include "main.h"

bool readToBuffer(char* buffer, int buffer_length, int addr, uint8_t conversion);
String readToString(int addr, uint8_t conversion);
bool writeFromString(uint16_t addr, int conversion, String value);

#endif //SOFTWARE_OPTOLINK_H
