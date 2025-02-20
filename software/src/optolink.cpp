//
// Created by chris on 7/10/23.
//
#include "optolink.h"

#include <utility>

bool link_locked = false;

const VitoWiFi::PacketVS2* result = nullptr;
VitoWiFi::OptolinkResult error = VitoWiFi::OptolinkResult::TIMEOUT;
bool hasResult = false;
bool hasError = false;

void onResponse(const VitoWiFi::PacketVS2& response, const VitoWiFi::Datapoint& request) {
    result = &response;
    hasResult = true;
}

void onError(VitoWiFi::OptolinkResult error_, const VitoWiFi::Datapoint& request) {
    error = error_;
    hasError = true;
}

void resetForCommunication() {
    getOptolink()->onError(onError);
    getOptolink()->onResponse(onResponse);
    result = nullptr;
    hasResult = false;
    hasError = false;
}

bool readToBufferUnsynchronized(char* buffer, VitoWiFi::Datapoint datapoint) {
    unsigned long start = millis();

    resetForCommunication();

    if (!getOptolink()->read(datapoint)) {
        strcpy(buffer, "SEND_ERROR");
        return false;
    }

    while (true) {
        getOptolink()->loop();
        if (hasResult || hasError) break;
        if ((millis() - start) > 5000UL) {
            strcpy(buffer, "READ_TIMEOUT");
            return false;
        }
        delay(10);
    }
    if (hasResult) {
        float val = datapoint.decode(*result);
        sprintf(buffer, "%.2f", val);
        return true;
    } else if (hasError) {
        if (error == VitoWiFi::OptolinkResult::TIMEOUT) {
            strcpy(buffer, "timeout");
        } else if (error == VitoWiFi::OptolinkResult::LENGTH) {
            strcpy(buffer, "length_error");
        } else if (error == VitoWiFi::OptolinkResult::NACK) {
            strcpy(buffer, "NACK");
        } else if (error == VitoWiFi::OptolinkResult::CRC) {
            strcpy(buffer, "CRC");
        } else if (error == VitoWiFi::OptolinkResult::ERROR) {
            strcpy(buffer, "ERROR");
        } else if (error == VitoWiFi::OptolinkResult::CONTINUE) {
            strcpy(buffer, "CONTINUE");
        } else if (error == VitoWiFi::OptolinkResult::PACKET) {
            strcpy(buffer, "PACKET");
        } else {
            strcpy(buffer, "UNKNOWN_ERROR");
        }
        return false;
    } else {
        strcpy(buffer, "no_result");
        return false;
    }
}

bool readToBuffer(char* buffer, VitoWiFi::Datapoint datapoint) {
    while (link_locked) delay(1);
    link_locked = true;
    bool ret = readToBufferUnsynchronized(buffer, datapoint);
    link_locked = false;
    return ret;
}
bool readToBuffer(char* buffer, uint16_t addr, uint8_t len, VitoWiFi::Converter *converter) {
    VitoWiFi::Datapoint datapoint = VitoWiFi::Datapoint("tmp", addr, len, *converter);
    return readToBuffer(buffer, datapoint);
}

bool readToStringLock = false;
String readToString(VitoWiFi::Datapoint datapoint) {
    // lock to prevent buffer from being used simultaneously
    while (readToStringLock) delay(1);
    readToStringLock = true;
    char buffer[25];
    readToBuffer(buffer, datapoint);
    readToStringLock = false;
    return buffer;
}

String readToString(uint16_t addr, uint8_t len, VitoWiFi::Converter *converter) {
    VitoWiFi::Datapoint readDatatpoint = VitoWiFi::Datapoint("tmp", addr, len, *converter);
    return readToString(readDatatpoint);
}
bool writeFromStringUnsynchronized(VitoWiFi::Datapoint datapoint, String value, char* readTo) {
    unsigned long start = millis();

    resetForCommunication();

    bool canWrite = false;
    uint16_t addr = datapoint.address();
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
        if (readTo != nullptr) strcpy(readTo, "INVALID_ADDRESS");
        return true;
    }

    String ret = "";

    if (!getOptolink()->write(datapoint, value.c_str())) {
        if (readTo != nullptr) strcpy(readTo, "FAILED_TO_SEND");
        return false;
    }

    while (millis() - 3000UL < start) {
        getOptolink()->loop();
        if (hasError || hasResult) break;
        delay(10);
    }
    if (!hasResult) {
        if (readTo != nullptr) strcpy(readTo, "TIMEOUT");
        return false;
    }

    if (readTo != nullptr) {
        return readToBufferUnsynchronized(readTo, datapoint);
    } else {
        return true;
    }
}
bool writeFromStringUnsynchronized(VitoWiFi::Datapoint datapoint, String value) {
    return writeFromStringUnsynchronized(datapoint, value, nullptr);
}

bool writeFromString(VitoWiFi::Datapoint datapoint, String value, char* readTo) {
    while (link_locked) delay(1);
    link_locked = true;
    bool ret = writeFromStringUnsynchronized(datapoint, std::move(value), readTo);
    link_locked = false;
    return ret;
}
bool writeFromString(VitoWiFi::Datapoint datapoint, String value) {
    return writeFromString(datapoint, value, nullptr);
}
