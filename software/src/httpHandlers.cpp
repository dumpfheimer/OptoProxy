#include "httpHandlers.h"

void handleRoot() {
    server.send(200, "text/plain",
                "usage: /read?addr=0F43&len=2&conv=temp (len 0-4, conv: 0: raw, 1:temp, 2:temps, 3:stat, 4:count, 5:counts, 6:mode, 7:hours, 8:cop)");
}

void handleHandshakes() {
    server.send(200, "text/plain", String(getLinkHandshakes()));
}

bool getDatapointConfig(DatapointConfig *config) {
    if (config == nullptr) return false;
    if (!server.hasArg("addr")) {
        server.send(400, "text/plain", "addr missing");
        return false;
    }
    config->addr = strtoul(server.arg("addr").c_str(), nullptr, 16);
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

void handleRead() {
    DatapointConfig *config;
    config = (DatapointConfig*) malloc(sizeof(DatapointConfig));
    // config is null checked in getDatapointConfig
    if (!getDatapointConfig(config)) {
        free(config);
        return;
    }

    if (server.hasArg("debug")) {
        server.send(200, "text/plain", String(config->addr) + ":" + String(config->len));
        free(config);
        return;
    }

    char *httpBuffer = (char*) malloc(sizeof(char) * HTTP_BUFFER_SIZE);
    if (httpBuffer == nullptr) {
        server.send(500, "text/plain", "OUT_OF_MEMORY");
    } else {
        snprintf(httpBuffer, HTTP_BUFFER_SIZE, "NORES");
        if (readToBuffer(httpBuffer, HTTP_BUFFER_SIZE, config)) {
            server.send(200, "text/plain", httpBuffer);
        } else {
            server.send(500, "text/plain", httpBuffer);
        }
    }

    free(httpBuffer);
    free(config);
}

void handleWrite() {
    DatapointConfig *config;
    config = (DatapointConfig*) malloc(sizeof(DatapointConfig));
    // config is null checked in getDatapointConfig
    if (!getDatapointConfig(config)) {
        free(config);
        return;
    }

    if (!server.hasArg("val")) {
        server.send(500, "text/plain", "val not set");
        free(config);
        return;
    }

    char *httpBuffer = (char*) malloc(sizeof(char) * HTTP_BUFFER_SIZE);
    if (httpBuffer == nullptr) {
        server.send(500, "text/plain", "OUT_OF_MEMORY");
    } else {
        strlcpy(httpBuffer, "NORES", HTTP_BUFFER_SIZE);
        bool forceWrite = false;
#ifdef FORCE_WRITE_PASSWORD
        if (server.hasArg("pw")) {
            if (server.arg("pw") == FORCE_WRITE_PASSWORD) {
                forceWrite = true;
            }
        }
#endif
        if (writeFromString(&server.arg("val"), httpBuffer, HTTP_BUFFER_SIZE, config, forceWrite)) {
            server.send(200, "text/plain", httpBuffer);
        } else {
            server.send(500, "text/plain", httpBuffer);
        }
    }

    free(httpBuffer);
    free(config);
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
        server.send(400, "text/plain", "val missing");
        return;
    }
    long pin = server.arg("pin").toInt();
    long val = server.arg("val").toInt();

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

    server.send(200, "text/plain", "ok");
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
    server.on("/handshakes", handleHandshakes);
    server.on("/read", handleRead);
    server.on("/write", handleWrite);
    server.on("/gpio/write", handleGpioWrite);
}
