//
// Created by chris on 21.02.23.
//

#ifndef SOFTWARE_MAIN_H
#define SOFTWARE_MAIN_H

// define the communication standard. can be OptolinkP300 or OptolinkKP
// check https://github.com/bertmelis/VitoWiFi for more information

// We are using a WEMOS D1

#include <ElegantOTA.h>

#include "configuration.h"
#include "wifi_mgr.h"

#ifdef ESP8266
#include <SoftwareSerial.h>
// CONFIGURATION FOR ESP8266
#define LOGGING_SERIAL Serial
#define OPTOLINK_SERIAL softwareSerial
extern SoftwareSerial softwareSerial;
#define OPTOLINK_SERIAL_RX D4
#define OPTOLINK_SERIAL_TX D0
#endif
#ifdef ESP32
#define OPTOLINK_SERIAL_RX 3
#define OPTOLINK_SERIAL_TX 1
#define LOGGING_SERIAL Serial
#define OPTOLINK_SERIAL Serial1
#endif
#ifndef WIFI_SSID
#include "wifi_mgr_portal.h"
#endif

#include "logging.h"

#define OPTOLINK_CLASS VitoWiFi::VitoWiFi<VitoWiFi::VS2>

extern XWebServer server;

#include "httpHandlers.h"

#endif //SOFTWARE_MAIN_H
