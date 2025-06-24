//
// Created by chris on 7/10/23.
//

#ifndef SOFTWARE_OPTOLINK_H
#define SOFTWARE_OPTOLINK_H

#include <Arduino.h>
#include "main.h"

#define OPTOLINK_DATA_SIZE 30

enum OptolinkTelegramError {
    NONE,
    TIMEOUT,
    CRC_ERROR
};

enum OptolinkCommand {
    PING_REQUEST = 0x05,
    RESET_COMMUNICATION = 0x04,
    BEGIN_COMMUNICATION = 0x16,
    ACK = 0x06,
    NACK = 0x15,
    DATA_REQUEST = 0x41
};

enum OptolinkDataRequestType {
    REQUEST = 0x00,
    RESPONSE = 0x01,
    RNACK = 0x02,
    ERROR = 0x03
};

enum OptolinkDataRequestAction {
    READ = 0x01,
    WRITE = 0x02,
    RPC = 0x07
};

class OptolinkTelegram {
public:
    OptolinkTelegram();
    void reset();
    void setError(OptolinkTelegramError error);
    OptolinkTelegramError getError();
    void setCmd(uint8_t cmd);
    uint8_t getCmd() const;
    void setLen(uint8_t len);
    uint8_t getLen() const;
    void pushData(uint8_t d);
    uint8_t getDataLength() const;
    uint8_t* getData();
    void setCrc(uint8_t crc);
    uint8_t getCrc() const;
    void calculateCrc();
    uint8_t getCalculatedCrc();
    bool crcIsValid();
    bool crcEquals(uint8_t crc) const;
    void writeTo(Stream& s);
    uint8_t cmd;
private:
    OptolinkTelegramError error;
    uint8_t len;
    uint8_t data[OPTOLINK_DATA_SIZE]{0};
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
bool optolinkIsLocked();
bool readToBuffer(char* buffer, uint16_t buffer_len, DatapointConfig *config);
bool writeFromString(const String *value, char* buffer, uint16_t buffer_len, DatapointConfig *config);

#endif //SOFTWARE_OPTOLINK_H
