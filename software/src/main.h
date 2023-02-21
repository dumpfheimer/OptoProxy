//
// Created by chris on 21.02.23.
//

#ifndef SOFTWARE_MAIN_H
#define SOFTWARE_MAIN_H

// define the communication standard. can be OptolinkP300 or OptolinkKP
// check https://github.com/bertmelis/VitoWiFi for more information

// We are using a WEMOS D1

#define OPTOLINK_CLASS OptolinkP300

#include <OptolinkP300.hpp>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#endif
#include <ElegantOTA.h>

#include "wifi-credentials.h"

// pin 2->D9
// pin 0->D8
#define OPTOLINK_OUT 0//D8
#define OPTOLINK_IN 2 //D9

#ifdef ESP8266
#define CONFIGURED

// CONFIGURATION FOR ESP8266
#define OPTOLINK_LOOP_THREADED false
#define OPTOLINK_LOOP_CORE 0
#define HTTPSERVER_LOOP_THREADED false
#define HTTPSERVER_LOOP_CORE 0
#define LOGGING_SERIAL SoftwareSerial


#endif
#ifdef ESP32
#define CONFIGURED
#define OPTOLINK_LOOP_THREADED true
#define OPTOLINK_LOOP_CORE 1
#define OPTOLINK_SERIAL_RX 3
#define OPTOLINK_SERIAL_TX 1
#define HTTPSERVER_LOOP_THREADED false
#define HTTPSERVER_LOOP_CORE 0
#define LOGGING_SERIAL HardwareSerial

#endif
#ifndef CONFIGURED
#define OPTOLINK_LOOP_THREADED false
#define OPTOLINK_LOOP_CORE 0
#define HTTPSERVER_LOOP_THREADED false
#define HTTPSERVER_LOOP_CORE 0

#endif

#include "httpHandlers.h"
#include "logging.h"

#ifdef ESP8266
extern ESP8266WebServer server;
#endif
#ifdef ESP32
extern WebServer server;
#endif


#if OPTOLINK_LOOP_THREADED == true
TaskHandle_t optolinkTaskHandle;
#endif

#if HTTPSERVER_LOOP_THREADED == true
TaskHandle_t httpserverTaskHandle;
#endif

OPTOLINK_CLASS* getOptolink();

#endif //SOFTWARE_MAIN_H
