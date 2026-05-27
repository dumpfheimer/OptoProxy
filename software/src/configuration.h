//
// Created by chris on 19/02/25.
//

#ifndef SOFTWARE_CONFIGURATION_H
#define SOFTWARE_CONFIGURATION_H

#if defined __has_include && __has_include ("my_config.h")
#include "my_config.h"
#endif

//#define USE_LOGGING

#ifndef STARTUP_QUIET_TIME
// do not talk to heater for 10s on power up. this is so the therme can start up normally. it gets stuck if optolink is used too early.
#define STARTUP_QUIET_TIME 10000
#endif

#endif //SOFTWARE_CONFIGURATION_H
