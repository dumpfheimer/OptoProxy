//
// Created by chris on 21.02.23.
//

#ifndef SOFTWARE_LOGGING_H
#define SOFTWARE_LOGGING_H

#include <Arduino.h>

#define USE_3LOGGING
#define LOGGING_RX 0 // D8
#define LOGGING_TX 2 // D9

#include "main.h"

#ifdef USE_LOGGING
#include <SoftwareSerial.h>
SoftwareSerial loggingSerial(LOGGING_RX, LOGGING_TX);
#endif

void setupLogging();
void print(String s);
void print(float f);
void println(String s);
void println(float f);
void println();

#ifdef USE_LOGGING
bool useLogging();
LOGGING_SERIAL* getLogger();
#else
bool useLogging();
LOGGING_SERIAL* getLogger();
#endif

#endif //SOFTWARE_LOGGING_H
