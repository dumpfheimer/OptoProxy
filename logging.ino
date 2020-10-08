#define USE_3LOGGING
#define LOGGING_RX 0 // D8
#define LOGGING_TX 2 // D9  

#ifdef USE_LOGGING
#include <SoftwareSerial.h>
SoftwareSerial loggingSerial(LOGGING_RX, LOGGING_TX);
#endif

void setupLogging() {
  #ifdef USE_LOGGING
    loggingSerial.begin(4800);
    delay(10);
  #endif
}

void print(String s) {
  #ifdef USE_LOGGING
    loggingSerial.print(s);
  #endif
}

void print(float f) {
  #ifdef USE_LOGGING
    loggingSerial.print(f);
  #endif
}


void println(String s) {
  #ifdef USE_LOGGING
    loggingSerial.println(s);
  #endif
}


void println(float f) {
  #ifdef USE_LOGGING
    loggingSerial.println(f);
  #endif
}


void println() {
  #ifdef USE_LOGGING
    loggingSerial.println();
  #endif
}

#ifdef USE_LOGGING
bool useLogging() {
  return true;
}

SoftwareSerial* getLogger() {
  return &loggingSerial;
}
#else
bool useLogging() {
  return false;
}

SoftwareSerial* getLogger() {
  return NULL;
}
#endif
