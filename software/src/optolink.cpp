//
// Created by chris on 7/10/23.
//
#include "optolink.h"

void readToBuffer(char* buffer, int buffer_length, int addr, uint8_t conversion) {
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

    if (getOptolink()->isBusy()) {
        strcpy("LINK_BUSY", buffer);
        return;
    }

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

    uint8_t value[16] = {0};

    getOptolink()->readFromDP(addr, len);

    while (true) {
        if (getOptolink()->available() > 0) {
            getOptolink()->read(value);

            /*for (uint8_t i = 0; i <= len && i < 16; ++i) {
                ltoa(value[i], buffer, 10);
                //return;
                //ret += String(value[i]);
                //Serial1.print(value[i], HEX);
            }*/
            break;
        } else if (getOptolink()->available() < 0) {
            ltoa(getOptolink()->readError(), buffer, 10);
            return;
        } else {
            getOptolink()->loop();
        }
        if (timeout < millis()) {
            strcpy("READ_TIMEOUT", buffer);
            return;
        }
        delay(10);
    }
    if (getOptolink()->available() < 0) {
        println(getOptolink()->readError());
    }

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
    dpv.getString(buffer, buffer_length);
}

String readToString(int addr, uint8_t conversion) {
    char buffer[25];
    readToBuffer(buffer, 25, addr, conversion);
    return buffer;
}