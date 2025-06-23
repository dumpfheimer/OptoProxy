//
// Created by chris on 21.02.23.
//

#ifndef SOFTWARE_LOGGING_H
#define SOFTWARE_LOGGING_H

#include <Arduino.h>

#define LOGGING_RX 0 // D8
#define LOGGING_TX 2 // D9

#include "main.h"

#ifdef USE_LOGGING
#include <SoftwareSerial.h>
//SoftwareSerial loggingSerial(LOGGING_RX, LOGGING_TX);
#endif

void setupLogging();
void print(const String& s);
void print(double f);
void println(const String& s);
void println(double f);
void println();

#ifdef USE_LOGGING
bool useLogging();
Stream* getLogger();
#else
bool useLogging();
Stream* getLogger();
#endif

#endif //SOFTWARE_LOGGING_H
