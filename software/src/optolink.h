//
// Created by chris on 7/10/23.
//

#ifndef SOFTWARE_OPTOLINK_H
#define SOFTWARE_OPTOLINK_H

#include <Arduino.h>
#include "main.h"

bool readToBuffer(char* buffer, float *val, uint16_t addr, uint8_t expectBytes, uint16_t factor);
bool writeFromString(const String& value, char* buffer, uint16_t addr, uint8_t expectBytes, uint16_t factor);

#endif //SOFTWARE_OPTOLINK_H
