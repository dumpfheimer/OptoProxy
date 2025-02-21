#include "logging.h"

#ifdef USE_LOGGING
#include <SoftwareSerial.h>
//SoftwareSerial loggingSerial(LOGGING_RX, LOGGING_TX);
#endif

void setupLogging() {
  #ifdef USE_LOGGING
    LOGGING_SERIAL.begin(9600);
    delay(10);
  #endif
}

void print(String s) {
  #ifdef USE_LOGGING
    LOGGING_SERIAL.print(s);
  #endif
}

void print(float f) {
  #ifdef USE_LOGGING
    LOGGING_SERIAL.print(f);
  #endif
}


void println(String s) {
  #ifdef USE_LOGGING
    LOGGING_SERIAL.println(s);
  #endif
}


void println(float f) {
  #ifdef USE_LOGGING
    LOGGING_SERIAL.println(f);
  #endif
}


void println() {
  #ifdef USE_LOGGING
    LOGGING_SERIAL.println();
  #endif
}

#ifdef USE_LOGGING
bool useLogging() {
  return true;
}

Stream* getLogger() {
  return &LOGGING_SERIAL;
}
#else
bool useLogging() {
  return false;
}

Stream* getLogger() {
  return NULL;
}
#endif
