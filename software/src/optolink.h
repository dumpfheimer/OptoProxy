//
// Created by chris on 7/10/23.
//

#ifndef SOFTWARE_OPTOLINK_H
#define SOFTWARE_OPTOLINK_H

#include <Arduino.h>
#include <DPValue.hpp>
#include <Datapoint.hpp>
#include "main.h"

void readToBuffer(char* buffer, int buffer_length, int addr, uint8_t conversion);
String readToString(int addr, uint8_t conversion);

#endif //SOFTWARE_OPTOLINK_H
