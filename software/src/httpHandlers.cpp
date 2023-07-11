#include "httpHandlers.h"

void handleRoot() {
    server.send(200, "text/plain",
                "usage: /read?addr=0F43&len=2&conv=temp (len 0-4, conv: 0: raw, 1:temp, 2:temps, 3:stat, 4:count, 5:counts, 6:mode, 7:hours, 8:cop");
}

void handleRead() {
    uint16_t addr = 0;
    int conv = 0;

    for (uint8_t i = 0; i < server.args(); i++) {
        if (server.argName(i) == "addr") {
            addr = strtoul(server.arg(i).c_str(), NULL, 16);
        } else if (server.argName(i) == "conv") {
            if (server.arg(i) == "raw") {
                conv = 0;
            } else if (server.arg(i) == "temp") {
                // conv4_1_UL 4
                conv = 1;
            } else if (server.arg(i) == "temps") {
                // conv1_1_US 1
                conv = 2;
            } else if (server.arg(i) == "stat") {
                // conv1_1_B 1
                conv = 3;
            } else if (server.arg(i) == "count") {
                // conv4_1_UL 4
                conv = 4;
            } else if (server.arg(i) == "counts") {
                // conv2_1_UL 2
                conv = 5;
            } else if (server.arg(i) == "mode") {
                // conv1_1_US 1
                conv = 6;
            } else if (server.arg(i) == "hours") {
                // conv4_3600_F 4
                conv = 7;
            } else if (server.arg(i) == "cop") {
                // conv1_10_F 2
                conv = 8;
            } else {
                conv = server.arg(i).toInt();
            }
        }
    }

    if (!getOptolink()->connected()) {
        server.send(500, "text/plain", "NOT_CONNECTED");
    }

    String ret = readToString(addr, conv);
    server.send(200, "text/plain", ret);
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
    uint16_t addr = strtoul(server.arg("addr").c_str(), NULL, 16);;
    int conv = 0;
    String strValue = server.arg("val");

    if (server.arg("conv") == "raw") {
        conv = 0;
    } else if (server.arg("conv") == "temp") {
        // conv4_1_UL 4
        conv = 1;
    } else if (server.arg("conv") == "temps") {
        // conv1_1_US 1
        conv = 2;
    } else if (server.arg("conv") == "stat") {
        // conv1_1_B 1
        conv = 3;
    } else if (server.arg("conv") == "count") {
        // conv4_1_UL 4
        conv = 4;
    } else if (server.arg("conv") == "counts") {
        // conv2_1_UL 2
        conv = 5;
    } else if (server.arg("conv") == "mode") {
        // conv1_1_US 1
        conv = 6;
    } else if (server.arg("conv") == "hours") {
        // conv4_3600_F 4
        conv = 7;
    } else if (server.arg("conv") == "cop") {
        // conv1_10_F 2
        conv = 8;
    } else {
        //conv = server.arg("conv").toInt();
        conv = strtoul(server.arg("conv").c_str(), NULL, 10);
    }

    if (!getOptolink()->connected()) {
        server.send(500, "text/plain", "NOT_CONNECTED");
    }

    bool success = writeFromString(addr, conv, strValue);
    if (success) {
        //server.send(200, "text/plain", "OK");
    } else {
        server.send(200, "text/plain", "ERROR_NOT_SET");
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
void handleAussenT() {
    String ret = readToString(0x0101, 1);
    server.send(200, "text/plain", ret);
}

void handleBrauchwasserT() {
    String ret = readToString(0x010D, 1);
    server.send(200, "text/plain", ret);
}

void handleRuecklaufT() {
    String ret = readToString(0x0106, 1);
    server.send(200, "text/plain", ret);
}

void handleVorlaufT() {
    String ret = readToString(0x0105, 1);
    server.send(200, "text/plain", ret);
}

void handleVorlaufTsoll() {
    String ret = readToString(0x1800, 1);
    server.send(200, "text/plain", ret);
}

void handleVentilHeizenWW() {
    String ret = readToString(0x494, 3);
    server.send(200, "text/plain", ret);
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

    server.begin();
}
