//
// Created by chris on 7/10/23.
//
#include "optolink.h"

bool link_locked = false;
uint8_t crc = 0;

bool readToBufferUnsynchronized(char* buffer, float *val, uint16_t addr, uint8_t expectBytes, uint16_t factor) {
    unsigned long start = millis();

    while (OPTOLINK_SERIAL.available()) OPTOLINK_SERIAL.read();

    OPTOLINK_SERIAL.write(0x41);
    OPTOLINK_SERIAL.write(0x05);
    crc = 0x05;
    OPTOLINK_SERIAL.write(0x00); crc = crc + 0x00;
    OPTOLINK_SERIAL.write(0x01); crc = crc + 0x01;
    OPTOLINK_SERIAL.write(addr >> 8); crc = crc + (addr >> 8);
    OPTOLINK_SERIAL.write(addr & 0xFF); crc = crc + (addr & 0xFF);
    OPTOLINK_SERIAL.write(0x02); crc = crc + 0x02;
    OPTOLINK_SERIAL.write(crc);

    while (true) {
        if (OPTOLINK_SERIAL.available()) break;
        if ((millis() - start) > 5000UL) {
            strcpy(buffer, "READ_TIMEOUT");
            return false;
        }
    }
    uint8_t serialBuffer[30] = {0};
    uint8_t len = 99;
    for (int i = 0; i < 30 && i < len + 5; i++) {
        OPTOLINK_SERIAL.read(&serialBuffer[i], 1);
        if (i == 3) len = serialBuffer[i];
    }
    if (serialBuffer[0] == 0x06 && serialBuffer[1] == 0x41) {
        crc = 0;
        for (int i = 0; i < len; i++) {
            crc += serialBuffer[i+2];
        }
        if (crc != serialBuffer[len + 4]) {
            strcpy(buffer, "CRC_MISMATCH");
            return false;
        }
        if (serialBuffer[3] == 0x01 && serialBuffer[4] == 0x01) {
            // answer to the read request
            uint16_t answerAddr = serialBuffer[5] << 8 | serialBuffer[6];
            if (answerAddr == addr) {
                // same address
                uint8_t len = serialBuffer[8];
                uint32_t data = 0;
                for (int i = 0; i < len; i++) {
                    data = data | serialBuffer[9+i];
                    if (i + 1 < len) data = data << 8;
                }
                float f = data;
                sprintf(buffer, "%.2f", f);
                return true;
            } else {
                strcpy(buffer, "ADDR_MISMATCH");
                return false;
            }
        } else {
            strcpy(buffer, "NOT_01_01");
            return false;
        }
    } else {
        sprintf(buffer, "NOT_06_41 %02X %02X", serialBuffer[0], serialBuffer[1]);
        return false;
    }
}

bool readToBuffer(char* buffer, float *val, uint16_t addr, uint8_t expectBytes, uint16_t factor) {
    while (link_locked) delay(1);
    link_locked = true;
    bool ret = readToBufferUnsynchronized(buffer, val,addr, expectBytes, factor);
    link_locked = false;
    return ret;
}

bool writeFromStringUnsynchronized(const String& value, char* buffer, uint16_t addr, uint8_t expectBytes, uint16_t factor) {
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
    if (addr == 0x2005) canWrite = true; // ramtemperaturregelung
    if (addr == 0x2006) canWrite = true; // heizkennlinie niveau
    if (addr == 0x2007) canWrite = true; // heizkennlinie steigung
    if (addr == 0x200A) canWrite = true; // Einfluss Raumtemperaturaufschaltung
    if (addr == 0x2034) canWrite = true; // Einfluss Raumtemperaturaufschaltung kühlen
    if (addr == 0xb000) canWrite = true; // betriebsmodus
    if (addr == 0x7002) canWrite = true; // temerpatur mittel langzeitermittlung min (counts)
    if (addr == 0x7003) canWrite = true; // Temperaturdifferenz heizen an = Langzeitmittel - 7003 - 2
    // Temperaturdifferenz heizen aus = Langzeitmittel - 7003 + 2
    if (addr == 0x7004) canWrite = true; // Temperaturdifferenz kühlgrenze Kühlgrenze = RaumSollTemp + 7004
    if (addr == 0x730F) canWrite = true; // Optimale Leistung bei min. Aussentemperatur
    if (addr == 0x7310) canWrite = true; // Optimale Leistung bei max. Aussentemperatur
    if (addr == 0x7414) canWrite = true; // Startleistung
    if (addr == 0x5006) canWrite = true; // Min. Pausenzeit Verdichter
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
        if (buffer != nullptr) strcpy(buffer, "INVALID_ADDRESS");
        return false;
    }

    return false;
}

bool writeFromString(const String& value, char* buffer, uint16_t addr, uint8_t expectBytes, uint16_t factor) {
    while (link_locked) delay(1);
    link_locked = true;
    bool ret = writeFromStringUnsynchronized(value, buffer, addr, expectBytes, factor);
    link_locked = false;
    return ret;
}
