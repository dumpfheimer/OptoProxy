#include "httpHandlers.h"

void handleRoot() {
    server.send(200, "text/plain",
                "usage: /read?addr=0F43&len=2&conv=temp (len 0-4, conv: 0: raw, 1:temp, 2:temps, 3:stat, 4:count, 5:counts, 6:mode, 7:hours, 8:cop");
}

char *httpBuffer = new char[60];
void handleRead() {
    if (!server.hasArg("addr")) {
        server.send(400, "text/plain", "addr missing");
        return;
    }
    uint16_t addr = strtoul(server.arg("addr").c_str(), NULL, 16);
    uint8_t len = 0;
    uint16_t factor = 1;

    if (server.hasArg("conv")) {
        if (server.arg("conv") == "raw") {
            len = 4;
        } else if (server.arg("conv") == "temp") {
            factor = 10;
            len = 2;
        } else if (server.arg("conv") == "temps" || server.arg("conv") == "percent") {
            len = 2;
        } else if (server.arg("conv") == "stat") {
            len = 1;
        } else if (server.arg("conv") == "count") {
            len = 4;
        } else if (server.arg("conv") == "counts") {
            len = 2;
        } else if (server.arg("conv") == "mode") {
            len = 1;
        } else if (server.arg("conv") == "hours") {
            factor = 3600;
            len = 4;
        } else if (server.arg("conv") == "cop") {
            factor = 10;
            len = 1;
        } else if (server.arg("conv") == "noconv") {
        } else if (server.arg("conv") == "div2") {
            factor = 2;
        } else if (server.arg("conv") == "div10") {
            factor = 10;
        } else if (server.arg("conv") == "div3600") {
            factor = 3600;
        }
    }

    if (server.hasArg("len")) len = server.arg("len").toInt();
    if (server.hasArg("length")) len = server.arg("length").toInt();
    if (len == 0) {
        server.send(500, "text/plain", "INVALID_LENGTH");
        return;
    }

    if (server.hasArg("debug")) {
        server.send(200, "text/plain", String(addr) + ":" + String(len));
        return;
    }

    if (httpBuffer == nullptr) {
        server.send(500, "text/plain" "OUT_OF_MEMORY");
    } else {
        float f = 999;
        if (readToBuffer(httpBuffer, &f, addr, len, factor)) {
            server.send(200, "text/plain", String(f));
        } else {
            server.send(500, "text/plain", httpBuffer);
        }
        delay(500);
    }
}

void handleWrite() {
    if (!server.hasArg("addr")) {
        server.send(400, "text/plain", "addr missing");
        return;
    }
    uint16_t addr = strtoul(server.arg("addr").c_str(), NULL, 16);
    uint8_t len = 0;
    uint16_t factor = 1;

    if (server.hasArg("conv")) {
        if (server.arg("conv") == "raw") {
            len = 4;
        } else if (server.arg("conv") == "temp") {
            factor = 10;
            len = 2;
        } else if (server.arg("conv") == "temps" || server.arg("conv") == "percent") {
            len = 2;
        } else if (server.arg("conv") == "stat") {
            len = 1;
        } else if (server.arg("conv") == "count") {
            len = 4;
        } else if (server.arg("conv") == "counts") {
            len = 2;
        } else if (server.arg("conv") == "mode") {
            len = 1;
        } else if (server.arg("conv") == "hours") {
            factor = 3600;
            len = 4;
        } else if (server.arg("conv") == "cop") {
            factor = 10;
            len = 1;
        } else if (server.arg("conv") == "noconv") {
        } else if (server.arg("conv") == "div2") {
            factor = 2;
        } else if (server.arg("conv") == "div10") {
            factor = 10;
        } else if (server.arg("conv") == "div3600") {
            factor = 3600;
        }
    }

    if (server.hasArg("len")) len = server.arg("len").toInt();
    if (server.hasArg("length")) len = server.arg("length").toInt();
    if (len == 0) {
        server.send(500, "text/plain", "INVALID_LENGTH");
        return;
    }

    if (server.hasArg("debug")) {
        server.send(200, "text/plain", String(addr) + ":" + String(len));
        return;
    }

    if (!server.hasArg("val")) {
        server.send(500, "text/plain", "val not set");
        return;
    }

    if (httpBuffer == nullptr) {
        server.send(500, "text/plain" "OUT_OF_MEMORY");
    } else {
        if (writeFromString(server.arg("val"), httpBuffer, addr, len, factor)) {
            server.send(200, "text/plain", httpBuffer);
        } else {
            server.send(500, "text/plain", httpBuffer);
        }
        delay(500);
    }
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
/*
void handleAussenT() {
    float f = 0;
    String ret = readToString(&f, 0x0101, 4, &VitoWiFi::div10);
    server.send(200, "text/plain", ret);
}

void handleBrauchwasserT() {
    float f = 0;
    String ret = readToString(&f, 0x010D, 4, &VitoWiFi::div10);
    server.send(200, "text/plain", ret);
}

void handleRuecklaufT() {
    float f = 0;
    String ret = readToString(&f, 0x0106, 4, &VitoWiFi::div10);
    server.send(200, "text/plain", ret);
}

void handleVorlaufT() {
    float f = 0;
    String ret = readToString(&f, 0x0105, 4, &VitoWiFi::div10);
    server.send(200, "text/plain", ret);
}

void handleVorlaufTsoll() {
    float f = 0;
    String ret = readToString(&f, 0x1800, 4, &VitoWiFi::div10);
    server.send(200, "text/plain", ret);
}

void handleVentilHeizenWW() {
    float f = 0;
    String ret = readToString(&f, 0x494, 1, &VitoWiFi::noconv);
    server.send(200, "text/plain", ret);
}*/

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
    /*
    server.on("/aussenT", handleAussenT);
    server.on("/brauchwasserT", handleBrauchwasserT);
    server.on("/ruecklaufT", handleRuecklaufT);
    server.on("/vorlaufT", handleVorlaufT);
    server.on("/vorlaufTsoll", handleVorlaufTsoll);
    server.on("/ventilHeizenWW", handleVentilHeizenWW);*/
    server.on("/gpio/write", handleGpioWrite);

    server.begin();
}
