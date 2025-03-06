//
// Created by chris on 21.02.23.
//

#ifndef SOFTWARE_HTTPHANDLERS_H
#define SOFTWARE_HTTPHANDLERS_H

#include <Arduino.h>

#include "main.h"
#include "optolink.h"

#define HTTP_BUFFER_SIZE 64

void setupHttp();
void loopHttp();

#endif //SOFTWARE_HTTPHANDLERS_H
