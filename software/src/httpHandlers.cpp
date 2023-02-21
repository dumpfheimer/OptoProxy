#include "httpHandlers.h"

void handleRoot() {
    server.send(200, "text/plain",
                "usage: /read?addr=0F43&len=2&conv=temp (len 0-4, conv: 0: raw, 1:temp, 2:temps, 3:stat, 4:count, 5:counts, 6:mode, 7:hours, 8:cop");
}

String readToString(int addr, uint8_t conversion) {
    unsigned long timeout = millis() + 15000UL;
    uint8_t len = 2;
    uint8_t valueCrap[16] = {0};

    if (getOptolink()->available() < 0) {
        println(getOptolink()->readError());
        // return "READ_ERROR";
    }
    if (getOptolink()->available() > 0) {
        getOptolink()->read(valueCrap);
        // return "READ_ERROR";
    }

    if (getOptolink()->isBusy()) return "LINK_BUSY";

    if (conversion == 1) {
        len = 4;
    } else if (conversion == 2) {
        len = 1;
    } else if (conversion == 3) {
        len = 1;
    } else if (conversion == 4) {
        len = 4;
    } else if (conversion == 5) {
        len = 2;
    } else if (conversion == 6) {
        len = 1;
    } else if (conversion == 7) {
        len = 4;
    } else if (conversion == 8) {
        len = 2;
    }

    if (len > 4) len = 4;
    if (len < 1) len = 1;

    String ret = "";
    uint8_t value[16] = {0};

    getOptolink()->readFromDP(addr, len);

    while (true) {
        if (getOptolink()->available() > 0) {
            getOptolink()->read(value);

            for (uint8_t i = 0; i <= len && i < 16; ++i) {
                ret += String(value[i]);
                //Serial1.print(value[i], HEX);
            }
            break;
        } else if (getOptolink()->available() < 0) {
            return String(getOptolink()->readError());
        } else {
            getOptolink()->loop();
        }
        if (timeout < millis()) {
            return "READ_TIMEOUT";
        }
        delay(10);
    }
    if (getOptolink()->available() < 0) {
        println(getOptolink()->readError());
    }
    char retBuff[25] = "";
    DPValue dpv(false);
    if (conversion == 1) {
        dpv = DPTemp("tmp", "tmp", addr, false).setLength(len).decode(&value[0]);
    } else if (conversion == 2) {
        dpv = DPTempS("tmp", "tmp", addr, false).setLength(len).decode(&value[0]);
    } else if (conversion == 3) {
        dpv = DPStat("tmp", "tmp", addr, false).setLength(len).decode(&value[0]);
    } else if (conversion == 4) {
        dpv = DPCount("tmp", "tmp", addr, false).setLength(len).decode(&value[0]);
    } else if (conversion == 5) {
        dpv = DPCountS("tmp", "tmp", addr, false).setLength(len).decode(&value[0]);
    } else if (conversion == 6) {
        dpv = DPMode("tmp", "tmp", addr, false).setLength(len).decode(&value[0]);
    } else if (conversion == 7) {
        dpv = DPHours("tmp", "tmp", addr, false).setLength(len).decode(&value[0]);
    } else if (conversion == 8) {
        dpv = DPCoP("tmp", "tmp", addr, false).setLength(len).decode(&value[0]);
    } else {
        dpv = DPRaw("tmp", "tmp", addr, false).setLength(len).decode(&value[0]);
    }
    dpv.getString(retBuff, sizeof(retBuff));
    return retBuff;
}

bool writeFromString(uint16_t addr, int conversion, String value) {
    unsigned long start = millis();
    uint8_t valueCrap[16] = {0};

    bool canWrite = false;
    if (addr == 0x7100) canWrite = true; // kühlart
    if (addr == 0x7101) canWrite = true; // heizkreis
    if (addr == 0x7102) canWrite = true; // raumtemp soll kuehlen
    if (addr == 0x7103) canWrite = true; // min vorlauftemp kühlen
    if (addr == 0x7104) canWrite = true; // einfluss raumtemp kühlen
    if (addr == 0x7106) canWrite = true; // witterungs / raumtemperaturgeführte kühlung
    if (addr == 0x7107) canWrite = true; // welcher heizkreis / tempsensor für kühlung
    if (addr == 0x7110) canWrite = true; // kühlkennlinie neigung
    if (addr == 0x7111) canWrite = true; // kühlkennlinie steigung
    if (addr == 0x2001) canWrite = true; // raumtemp red soll
    if (addr == 0x2003) canWrite = true; // use remote control
    if (addr == 0x2000) canWrite = true; // raumtep soll
    if (addr == 0x2006) canWrite = true; // heizkennlinie niveau
    if (addr == 0x2007) canWrite = true; // heizkennlinie steigung
    if (addr == 0xb000) canWrite = true; // betriebsmodus
    if (addr == 0x7002) canWrite = true; // temerpatur mittel langzeitermittlung min (counts)
    if (addr == 0x7003) canWrite = true; // Temperaturdifferenz heizen an = Langzeitmittel - 7003 - 2
    // Temperaturdifferenz heizen aus = Langzeitmittel - 7003 + 2
    if (addr == 0x7004) canWrite = true; // Temperaturdifferenz kühlgrenze Kühlgrenze = RaumSollTemp + 7004
    // 6000 WW Soll
    // B020 1x WW bereiten
    // 600C WW2 Soll
    // to start: http://192.168.11.30/write?addr=0xB020&len=1&val=true&stat=1
    if (addr == 0xB020) canWrite = true; // 1x WW bereiten
    if (addr == 0x6000) canWrite = true; // 1x WW Soll
    if (addr == 0x600C) canWrite = true; // 1x WW2 Soll

    // http://192.168.11.30/read?addr=0x1A52&conv=cop Ventilator PROZENT
    // http://192.168.11.30/read?addr=0x1A53&conv=cop Lüfter Prozent
    // http://192.168.11.30/read?addr=0x1A54&conv=cop Kompressor Prozent
    // http://192.168.11.30/read?addr=0x1AC3&conv=cop Verdichter Last Prozent
    if ((addr & 0xFFF0) == 0x01D0) canWrite = true;

    // kühlen langzeitermittlung: 30 min
    //                            7004: 10 //bei RaumSollTemp + 1 kühlen
    // heizen langzeitermittlung: 180min

    if (!canWrite) {
        server.send(200, "text/plain", "INVALID_ADDRESS");
        return true;
    }

    if (getOptolink()->available() < 0) {
        println(getOptolink()->readError());
    }
    if (getOptolink()->available() > 0) {
        getOptolink()->read(valueCrap);
    }

    if (getOptolink()->isBusy()) return false;

    String ret = "";

    /*
    0 raw ?
    1 temp 2_10_UL
    2 temps 1_1_US
    3 stat 1_1_B
    4 count 4_1_UL
    5 counts 2_1_UL
    6 mode 1_1_US
    7 hours 4_360_F
    8 cop 1_10_F
    */

    uint8_t* writeValues;
    size_t writeLength = 0;
    if (conversion == 1) {
        uint16_t tmp = (value.toFloat() * 10);
        conv2_1_UL dpType = conv2_1_UL();
        writeValues = (uint8_t*) malloc(sizeof(uint8_t) * dpType.getLength());
        DPValue dpValue(tmp);
        writeLength = dpType.getLength();
        dpType.encode(writeValues, dpValue);
    } else if (conversion == 2) {
        uint8_t tmp = value.toInt();
        conv1_1_US dpType = conv1_1_US();
        writeValues = (uint8_t*) malloc(sizeof(uint8_t) * dpType.getLength());
        DPValue dpValue(tmp);
        writeLength = dpType.getLength();
        dpType.encode(writeValues, dpValue);
    } else if (conversion == 3) {
        value.toLowerCase();
        bool tmp = (value == "true" || value == "1");
        conv1_1_B dpType = conv1_1_B();
        writeValues = (uint8_t*) malloc(sizeof(uint8_t) * dpType.getLength());
        DPValue dpValue(tmp);
        writeLength = dpType.getLength();
        dpType.encode(writeValues, dpValue);
    } else if (conversion == 4) {
        uint32_t tmp = value.toInt();
        conv4_1_UL dpType = conv4_1_UL();
        writeValues = (uint8_t*) malloc(sizeof(uint8_t) * dpType.getLength());
        DPValue dpValue(tmp);
        writeLength = dpType.getLength();
        dpType.encode(writeValues, dpValue);
    } else if (conversion == 5) {
        uint16_t tmp = value.toInt();
        conv2_1_UL dpType = conv2_1_UL();
        writeValues = (uint8_t*) malloc(sizeof(uint8_t) * dpType.getLength());
        DPValue dpValue(tmp);
        writeLength = dpType.getLength();
        dpType.encode(writeValues, dpValue);
    } else if (conversion == 6) {
        uint8_t tmp = value.toInt();
        conv1_1_US dpType = conv1_1_US();
        writeValues = (uint8_t*) malloc(sizeof(uint8_t) * dpType.getLength());
        DPValue dpValue(tmp);
        writeLength = dpType.getLength();
        dpType.encode(writeValues, dpValue);
    } else if (conversion == 7) {
        float tmp = value.toFloat();
        conv4_3600_F dpType = conv4_3600_F();
        writeValues = (uint8_t*) malloc(sizeof(uint8_t) * dpType.getLength());
        DPValue dpValue(tmp);
        writeLength = dpType.getLength();
        dpType.encode(writeValues, dpValue);
    } else {
        server.send(200, "text/plain", "INVALID_CONVERSION_2");
        return true;
    }

    /*{
        server.send(500, "text/plain", String(writeLength) +  String(writeValues[1]));
        return true;
    }*/

    if (!getOptolink()->writeToDP(addr, writeLength, writeValues)) {
        free(writeValues);
        server.send(500, "text/plain", "FAILED_TO_SEND");
        return true;
    }
    free(writeValues);

    while (getOptolink()->available() != 0 && millis() - 3000UL < start) {
        getOptolink()->loop();
        yield();
        //Serial.flush();
    }

    if (getOptolink()->available() > 0) {
        getOptolink()->read(valueCrap);
    }
    if (getOptolink()->available() < 0) {
        getOptolink()->readError();
        server.send(500, "text/plain", "WRITE_FAILED");
        return true;
    }

    if (!getOptolink()->readFromDP(addr, writeLength)) {
        server.send(500, "text/plain", "COULD_NOT_INITIALIZE_READ");
        return true;
    }

    uint8_t serialResultBuffer[32];
    char valueBuffer[25] = {0};
    char httpResultBuffer[100] = {0};
    //dpv.getString(valueBuffer, sizeof(valueBuffer));

    while (millis() - 3000UL < start) {
        int8_t available = getOptolink()->available();

        if (available > 0) {
            getOptolink()->read(serialResultBuffer);

            //sprintf(httpResultBuffer, "SUCCESS: %d %d %s %s %s", addr, dpv.getRawLength(), valueBuffer, serialResultBuffer, value.c_str());
            server.send(200, "text/plain", value);
            return true;
        } else if (available < 0) {
            uint8_t err = getOptolink()->readError();
            sprintf(httpResultBuffer, "WRITE ERROR: %d %d %s %d %d", addr, writeLength, valueBuffer, err, available);
            server.send(200, "text/plain", httpResultBuffer);
            return true;
        } else {
            getOptolink()->loop();
            yield();
            Serial.flush();
        }
    }
    if (getOptolink()->available() < 0) {
        uint8_t err = getOptolink()->readError();
        sprintf(httpResultBuffer, "WRITE_ERROR2: %d %d %d %s %d", addr, writeLength, conversion, valueBuffer, err);
        server.send(500, "text/plain", httpResultBuffer);
        return true;
    }
    server.send(500, "text/plain", "WRITE_TIMEOUT");
    return true;
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
