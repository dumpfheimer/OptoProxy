//
// Created by chris on 7/10/23.
//

#ifndef SOFTWARE_OPTOLINK_H
#define SOFTWARE_OPTOLINK_H

#include <Arduino.h>
#include "main.h"

enum OptolinkTelegramError {
    NONE,
    TIMEOUT,
    CRC_ERROR
};
class OptolinkTelegram {
public:
    OptolinkTelegram();
    void reset();
    void setError(OptolinkTelegramError error);
    OptolinkTelegramError getError();
    void setCmd(uint8_t cmd);
    uint8_t getCmd();
    void setLen(uint8_t len);
    uint8_t getLen();
    void pushData(uint8_t d);
    uint8_t getDataLength();
    uint8_t* getData();
    void setCrc(uint8_t crc);
    uint8_t getCrc();
    void calculateCrc();
    uint8_t getCalculatedCrc();
    bool crcIsValid();
    bool crcEquals(uint8_t crc);
    void writeTo(Stream& s);
    String toString();
    void print();
    uint8_t cmd;
private:
    OptolinkTelegramError error;
    uint8_t len;
    uint8_t data[30];
    uint8_t dataPtr;
    uint8_t crc;
};

struct DatapointConfig {
    uint16_t addr;
    uint8_t len;
    uint16_t factor;
    bool sign;
    bool hex;
    double val;
};

bool loopOptolink();
bool readToBuffer(char* buffer, DatapointConfig *config);
bool writeFromString(const String& value, char* buffer, DatapointConfig *config);

#endif //SOFTWARE_OPTOLINK_H
