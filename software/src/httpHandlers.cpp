#include "httpHandlers.h"

void handleRoot() {
    server.send(200, "text/plain",
                "usage: /read?addr=0F43&len=2&conv=temp (len 0-4, conv: 0: raw, 1:temp, 2:temps, 3:stat, 4:count, 5:counts, 6:mode, 7:hours, 8:cop");
}

void handleRead() {
    if (!server.hasArg("addr")) {
        server.send(400, "text/plain", "addr missing");
        return;
    }
    uint16_t addr = strtoul(server.arg("addr").c_str(), NULL, 16);
    uint16_t len = 0;

    VitoWiFi::Converter *converter = nullptr;
    if (server.hasArg("conv")) {
        if (server.arg("conv") == "raw") {
            converter = &VitoWiFi::noconv;
            len = 4;
        } else if (server.arg("conv") == "temp") {
            // DPTemp
            converter = &VitoWiFi::div10;
            len = 2;
        } else if (server.arg("conv") == "temps" || server.arg("conv") == "percent") {
            // conv1_1_US 1 DPTempS
            converter = &VitoWiFi::noconv;
            len = 2;
        } else if (server.arg("conv") == "stat") {
            // conv1_1_B 1 DPStat
            converter = &VitoWiFi::noconv;
            len = 1;
        } else if (server.arg("conv") == "count") {
            // conv4_1_UL 4 DPCount
            converter = &VitoWiFi::noconv;
            len = 4;
        } else if (server.arg("conv") == "counts") {
            // conv2_1_UL 2 DPCountS
            converter = &VitoWiFi::noconv;
            len = 2;
        } else if (server.arg("conv") == "mode") {
            // conv1_1_US 1 DPMode
            converter = &VitoWiFi::noconv;
            len = 1;
        } else if (server.arg("conv") == "hours") {
            // conv4_3600_F 4 DPHours
            converter = &VitoWiFi::div3600;
            len = 4;
        } else if (server.arg("conv") == "cop") {
            // conv1_10_F 2 DPCoP
            converter = &VitoWiFi::div10;
            len = 1;
        } else if (server.arg("conv") == "noconv") {
            converter = &VitoWiFi::noconv;
        } else if (server.arg("conv") == "div2") {
            converter = &VitoWiFi::div2;
        } else if (server.arg("conv") == "div10") {
            converter = &VitoWiFi::div10;
        } else if (server.arg("conv") == "div3600") {
            converter = &VitoWiFi::div3600;
        }
    }
    if (converter == nullptr) {
        server.send(500, "text/plain", "NO_CONVERTER");
        return;
    }
    if (server.hasArg("len")) len = server.arg("len").toInt();
    if (server.hasArg("length")) len = server.arg("length").toInt();
    VitoWiFi::Datapoint readDatatpoint = VitoWiFi::Datapoint("tmp", addr, len, *converter);

    if (server.hasArg("debug")) {
        server.send(200, "text/plain", String(addr) + ":" + String(len));
        return;
    }

    char *buff = (char*) malloc(sizeof(char) * 25);
    if (buff == nullptr) {
        server.send(500, "text/plain" "OUT_OF_MEMORY");
    } else {
        if (readToBuffer(buff, readDatatpoint)) {
            server.send(200, "text/plain", buff);
        } else {
            server.send(500, "text/plain", buff);
        }
        free(buff);
    }
}

void handleWrite() {
    if (!server.hasArg("addr")) {
        server.send(400, "text/plain", "addr missing");
        return;
    }
    if (!server.hasArg("conv")) {
        server.send(400, "text/plain", "conv missing");
        return;
    }
    if (!server.hasArg("val")) {
        server.send(400, "text/plain", "val missing");
        return;
    }
    uint16_t addr = strtoul(server.arg("addr").c_str(), NULL, 16);
    uint16_t len = 0;
    String strValue = server.arg("val");

    VitoWiFi::Converter *converter = nullptr;
    if (server.hasArg("conv")) {
        if (server.arg("conv") == "raw") {
            converter = &VitoWiFi::noconv;
            len = 4;
        } else if (server.arg("conv") == "temp") {
            converter = &VitoWiFi::noconv;
            len = 4;
        } else if (server.arg("conv") == "temps" || server.arg("conv") == "percent") {
            converter = &VitoWiFi::noconv;
            len = 1;
        } else if (server.arg("conv") == "stat") {
            converter = &VitoWiFi::noconv;
            len = 1;
        } else if (server.arg("conv") == "count") {
            converter = &VitoWiFi::noconv;
            len = 4;
        } else if (server.arg("conv") == "counts") {
            converter = &VitoWiFi::noconv;
            len = 2;
        } else if (server.arg("conv") == "mode") {
            converter = &VitoWiFi::noconv;
            len = 1;
        } else if (server.arg("conv") == "hours") {
            converter = &VitoWiFi::div3600;
            len = 4;
        } else if (server.arg("conv") == "cop") {
            len = 1;
        } else if (server.arg("conv") == "noconv") {
            converter = &VitoWiFi::noconv;
        } else if (server.arg("conv") == "div2") {
            converter = &VitoWiFi::div2;
        } else if (server.arg("conv") == "div10") {
            converter = &VitoWiFi::div10;
        } else if (server.arg("conv") == "div3600") {
            converter = &VitoWiFi::div3600;
        }
    }
    if (converter == nullptr) {
        server.send(500, "text/plain", "NO_CONVERTER");
        return;
    }
    VitoWiFi::Datapoint datapoint = VitoWiFi::Datapoint("tmp", addr, len, *converter);

    char *resultBuffer = new char[32];
    bool success = writeFromString(datapoint, strValue, resultBuffer);
    if (success) {
        server.send(200, "text/plain", resultBuffer);
    } else {
        server.send(200, "text/plain", "ERROR_NOT_SET");
    }
    delete resultBuffer;
}


/*
  server.on("/", handleRoot);
  server.on("/read", handleRead);
  server.on("/aussenT", handleAussenT);
  server.on("/brauchwasserT", handleBrauchwasserT);
  server.on("/ruecklaufT", handleRuecklaufT);
  server.on("/vorlaufT", handleVorlaufT);
  server.on("/vorlaufTsoll", handleVorlaufTsoll);
  server.on("/ventilHeizenWW", handleVentilHeizenWW);
DPTemp aussenT("aussenT", "boiler", 0x0101); // Aussentemperatur (Sensor 1)
DPTemp brauchwasserT("brauchwasserT", "boiler", 0x010D); // Brauchwassertemperatur
DPTemp ruecklaufT("ruecklaufT", "boiler", 0x0106); // Rücklauftemperatur (17A)
DPTemp vorlaufT("vorlaufT", "boiler", 0x0105); // Vorlauftemperatur (17B)
DPTemp vorlaufTsoll("vorlaufTsoll", "boiler", 0x1800); // Vorlaufsolltemperatur Anlage
DPStat ventilHeizenWW("ventilHeizenWW", "boiler", 0x494); // Status Umschaltventil Heizen/Warmwasser Ein/Aus

#define DATAPOINTS*/
void handleAussenT() {
    String ret = readToString(0x0101, 4, &VitoWiFi::div10);
    server.send(200, "text/plain", ret);
}

void handleBrauchwasserT() {
    String ret = readToString(0x010D, 4, &VitoWiFi::div10);
    server.send(200, "text/plain", ret);
}

void handleRuecklaufT() {
    String ret = readToString(0x0106, 4, &VitoWiFi::div10);
    server.send(200, "text/plain", ret);
}

void handleVorlaufT() {
    String ret = readToString(0x0105, 4, &VitoWiFi::div10);
    server.send(200, "text/plain", ret);
}

void handleVorlaufTsoll() {
    String ret = readToString(0x1800, 4, &VitoWiFi::div10);
    server.send(200, "text/plain", ret);
}

void handleVentilHeizenWW() {
    String ret = readToString(0x494, 1, &VitoWiFi::noconv);
    server.send(200, "text/plain", ret);
}

#define MAX_TIMER_GPIO 20
unsigned long timer_start[MAX_TIMER_GPIO] = {0};
unsigned long timer_duration[MAX_TIMER_GPIO] = {0};
uint8_t timer_value[MAX_TIMER_GPIO] = {0};

void handleGpioWrite() {
    if (!server.hasArg("pin")) {
        server.send(400, "text/plain", "pin missing");
        return;
    }
    if (!server.hasArg("val")) {
        server.send(400, "text/plain", "pin missing");
        return;
    }
    int pin = server.arg("pin").toInt();
    int val = server.arg("val").toInt();

    if (server.hasArg("timer")) {
        if (pin < MAX_TIMER_GPIO) {
            unsigned long time = server.arg("timer").toInt();
            timer_start[pin] = millis();
            timer_duration[pin] = time;
            timer_value[pin] = val == 0 ? 1 : 0;
        } else {
            server.send(400, "text/plain", "gpio out of reach for timer");
        }
    }

    pinMode(pin, OUTPUT);
    digitalWrite(pin, val);

    server.send(200, "ok");
}

void loopHttp() {
    for (uint8_t i = 0; i < MAX_TIMER_GPIO; i++) {
        unsigned long t = millis();
        if (timer_start[i] != 0) {
            if ((t - timer_start[i]) >= timer_duration[i]) {
                digitalWrite(i, timer_value[i]);
                timer_start[i] = 0;
            }
        }
    }
}

void setupHttp() {
    server.on("/", handleRoot);
    server.on("/read", handleRead);
    server.on("/write", handleWrite);
    server.on("/aussenT", handleAussenT);
    server.on("/brauchwasserT", handleBrauchwasserT);
    server.on("/ruecklaufT", handleRuecklaufT);
    server.on("/vorlaufT", handleVorlaufT);
    server.on("/vorlaufTsoll", handleVorlaufTsoll);
    server.on("/ventilHeizenWW", handleVentilHeizenWW);
    server.on("/gpio/write", handleGpioWrite);

    server.begin();
}
