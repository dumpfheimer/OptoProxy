#include "httpHandlers.h"

void handleRoot() {
    server.send(200, "text/plain",
                "usage: /read?addr=0F43&len=2&conv=temp (len 0-4, conv: 0: raw, 1:temp, 2:temps, 3:stat, 4:count, 5:counts, 6:mode, 7:hours, 8:cop");
}
bool getDatapointConfig(DatapointConfig *config) {
    if (config == nullptr) return false;
    if (!server.hasArg("addr")) {
        server.send(400, "text/plain", "addr missing");
        return false;
    }
    config->addr = strtoul(server.arg("addr").c_str(), NULL, 16);
    config->len = 0;
    config->factor = 1;
    config->sign = false;
    config->hex = false;

    if (server.hasArg("hex")) config->hex = true;
    if (server.hasArg("conv")) {
        if (server.arg("conv") == "raw") {
            config->len = 4;
        } else if (server.arg("conv") == "temp") {
            config->factor = 10;
            config->len = 2;
        } else if (server.arg("conv") == "temps" || server.arg("conv") == "percent") {
            config->len = 2;
        } else if (server.arg("conv") == "stat") {
            config->len = 1;
        } else if (server.arg("conv") == "count") {
            config->len = 4;
        } else if (server.arg("conv") == "counts") {
            config->len = 2;
        } else if (server.arg("conv") == "mode") {
            config->len = 1;
        } else if (server.arg("conv") == "hours") {
            config->factor = 3600;
            config->len = 4;
        } else if (server.arg("conv") == "cop") {
            config->factor = 10;
            config->len = 1;
        } else if (server.arg("conv") == "noconv") {
        } else if (server.arg("conv") == "div2") {
            config->factor = 2;
        } else if (server.arg("conv") == "div10") {
            config->factor = 10;
        } else if (server.arg("conv") == "div3600") {
            config->factor = 3600;
        }
    }

    if (server.hasArg("len")) config->len = server.arg("len").toInt();
    if (server.hasArg("length")) config->len = server.arg("length").toInt();
    if (server.hasArg("sign") && server.arg("sign") == "true") config->sign = true;
    if (config->len == 0) {
        server.send(500, "text/plain", "INVALID_LENGTH");
        return false;
    }
    return true;
}

char *httpBuffer = new char[HTTP_BUFFER_SIZE];
void handleRead() {
    DatapointConfig *config;
    config = (DatapointConfig*) malloc(sizeof(DatapointConfig));
    if (!getDatapointConfig(config)) {
        free(config);
        return;
    }

    if (server.hasArg("debug")) {
        server.send(200, "text/plain", String(config->addr) + ":" + String(config->len));
        free(config);
        return;
    }

    if (httpBuffer == nullptr) {
        server.send(500, "text/plain" "OUT_OF_MEMORY");
    } else {
        if (readToBuffer(httpBuffer, HTTP_BUFFER_SIZE, config)) {
            server.send(200, "text/plain", httpBuffer);
        } else {
            server.send(500, "text/plain", httpBuffer);
        }
    }
    free(config);
}

void handleWrite() {
    DatapointConfig *config;
    config = (DatapointConfig*) malloc(sizeof(DatapointConfig));
    if (!getDatapointConfig(config)) {
        free(config);
        return;
    }

    if (!server.hasArg("val")) {
        server.send(500, "text/plain", "val not set");
        free(config);
        return;
    }

    if (httpBuffer == nullptr) {
        server.send(500, "text/plain" "OUT_OF_MEMORY");
    } else {
        if (writeFromString(server.arg("val"), httpBuffer, HTTP_BUFFER_SIZE, config)) {
            server.send(200, "text/plain", httpBuffer);
        } else {
            server.send(500, "text/plain", httpBuffer);
        }
    }
    free(config);
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
}
