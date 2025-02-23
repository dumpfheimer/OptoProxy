//
// Created by chris on 7/10/23.
//
#include "optolink.h"

volatile bool link_locked = false;
uint8_t crc = 0;

OptolinkTelegram::OptolinkTelegram() {
    this->reset();
}
void OptolinkTelegram::reset() {
    this->error = NONE;
    this->cmd = 0;
    this->len = 0;
    while (this->dataPtr > 0) this->data[this->dataPtr--] = 0;
    this->data[0] = 0;
    this->crc = 0;
}
void OptolinkTelegram::setError(OptolinkTelegramError error) {
    this->error = error;
}
OptolinkTelegramError OptolinkTelegram::getError() {
    return this->error;
}
void OptolinkTelegram::setCmd(uint8_t cmd) {
    this->cmd = cmd;
}
uint8_t OptolinkTelegram::getCmd() {
    return this->cmd;
}
void OptolinkTelegram::setLen(uint8_t len) {
    this->len = len;
}
uint8_t OptolinkTelegram::getLen() {
    return this->len;
}
void OptolinkTelegram::pushData(uint8_t d) {
    this->data[this->dataPtr++] = d;
}
uint8_t OptolinkTelegram::getDataLength() {
    return this->dataPtr;
}
uint8_t* OptolinkTelegram::getData() {
    return this->data;
}
void OptolinkTelegram::setCrc(uint8_t crc) {
    this->crc = crc;
}
uint8_t OptolinkTelegram::getCrc() {
    return this->crc;
}
void OptolinkTelegram::calculateCrc() {
    this->crc = getCalculatedCrc();
}
uint8_t OptolinkTelegram::getCalculatedCrc() {
    uint8_t crc = 0;
    crc += this->getDataLength();
    for (uint8_t i = 0; i < this->getDataLength(); i++) {
        crc += this->data[i];
    }
    return crc;
}
bool OptolinkTelegram::crcIsValid() {
    return this->getCalculatedCrc() == this->crc;
}
bool OptolinkTelegram::crcEquals(uint8_t crc) {
    return this->crc == crc;
}
void OptolinkTelegram::writeTo(Stream& s) {
    this->calculateCrc();
    s.write(this->cmd);
    s.write(this->getDataLength());
    for (int i = 0; i < this->getDataLength(); i++) s.write(this->data[i]);
    this->calculateCrc();
    s.write(this->crc);
}
String OptolinkTelegram::toString() {
    String ret = String(this->cmd, HEX) + " " + String(this->getDataLength(), HEX);
    for (uint8_t i = 0; i < this->getDataLength() && i < 10; i++) ret += ":" + String(this->data[i], HEX);
    if (this->getDataLength() > 10) ret += "...";
    ret += "=" + String(this->crc, HEX);
    return ret;
}
void OptolinkTelegram::print() {
    println(toString());
}

bool waitAvailable(Stream &s, unsigned long timeout) {
    if (s.available()) return true;
    unsigned long start = millis();
    while ((millis() - start) < timeout) {
        if (s.available()) return true;
        delay(10);
    }
    return s.available();
}

bool read(Stream &s, uint8_t *c, long timeout) {
    waitAvailable(s, timeout);
    if (s.available()) {
        OPTOLINK_SERIAL.read(c, 1);
        return true;
    } else {
        return false;
    }
}
bool read(Stream &s, uint8_t *c) {
    return read(s, c, 20);
}
void readTelegram(OptolinkTelegram* telegram, unsigned long timeout) {
    //println("readTelegram");
    if (telegram == nullptr) return;
    telegram->reset();

    uint8_t charBuffer;
    if (!read(OPTOLINK_SERIAL, &charBuffer, timeout)) {
        telegram->setError(TIMEOUT);
        return;
    }
    telegram->setCmd(charBuffer);
    if (telegram->getCmd() == PING_REQUEST) return;
    if (telegram->getCmd() == ACK) return;
    if (telegram->getCmd() == NACK) return;
    if (!read(OPTOLINK_SERIAL, &charBuffer)) {
        telegram->setError(TIMEOUT);
        return;
    }
    telegram->setLen(charBuffer);
    uint8_t bytes = telegram->getLen();
    for (uint8_t i = 0; i < bytes; i++) {
        if (!read(OPTOLINK_SERIAL, &charBuffer)) {
            telegram->setError(TIMEOUT);
            return;
        }
        telegram->pushData(charBuffer);
    }
    if (!read(OPTOLINK_SERIAL, &charBuffer)) {
        telegram->setError(TIMEOUT);
        return;
    }
    telegram->setCrc(charBuffer);
    if (!telegram->crcIsValid()) {
        telegram->setError(CRC_ERROR);
    }
    print("readTelegram finished");
    //telegram->print();
}
void readTelegram(OptolinkTelegram* telegram) {
    readTelegram(telegram, 500);
}

void readUsefulTelegram(OptolinkTelegram* telegram, unsigned long timeout) {
    telegram->reset();
    unsigned long start = millis();
    while ((millis() - start) < timeout) {
        readTelegram(telegram, timeout - (millis() - start));
        println(telegram->getCmd());

        if (telegram->getCmd() == PING_REQUEST) {
            println("got ping msg");
            // ping
            OPTOLINK_SERIAL.write(RESET_COMMUNICATION);
            // this seems bad
            // it only works after the ESP dies and restarts. then the heater will communicate again
            readTelegram(telegram, 2500);
            if (telegram->getCmd() == PING_REQUEST) {
                println("got second ping");
                telegram->reset();
                telegram->setCmd(BEGIN_COMMUNICATION);
                telegram->writeTo(OPTOLINK_SERIAL);
                readTelegram(telegram);
            } else {
                println("did not get second ping message");
            }
        } else if (telegram->getCmd() == ACK) {
            println("got ack msg");
            // ack
            //return readUsefulTelegram(telegram, timeout - (millis() - start));
        } else {
            return;
        }
    }
    telegram->setError(TIMEOUT);
}

bool loopOptolink() {
    if (!OPTOLINK_SERIAL.available()) return false;
    OptolinkTelegram loopTelegram;
    readTelegram(&loopTelegram);
    println("read a telegram in loop");
    print(loopTelegram.getCmd());
    if (loopTelegram.getCmd() == PING_REQUEST) {
        while (waitAvailable(OPTOLINK_SERIAL, 20)) OPTOLINK_SERIAL.read();
        OPTOLINK_SERIAL.write(RESET_COMMUNICATION);
        println("waiting for second telegram");
        waitAvailable(OPTOLINK_SERIAL, 2500);
        readTelegram(&loopTelegram);
        println("read second telegram");
        if (loopTelegram.getCmd() == PING_REQUEST) {
            println("was 05 too");
            while (waitAvailable(OPTOLINK_SERIAL, 20)) OPTOLINK_SERIAL.read();
            loopTelegram.reset();
            loopTelegram.setCmd(BEGIN_COMMUNICATION);
            loopTelegram.writeTo(OPTOLINK_SERIAL);
            readTelegram(&loopTelegram);
            println("read third telegram");
            if (loopTelegram.getCmd() == ACK) {
                println("was 06, handshake completed");
                while (waitAvailable(OPTOLINK_SERIAL, 20)) OPTOLINK_SERIAL.read();
                return true;
            }
        }
    }
    return false;
}

bool convertData(DatapointConfig *config, uint8_t dataBuffer[]) {
    config->val = 0;
    if (config->len == 1) {
        if (config->sign) {
            int8_t d = 0 | dataBuffer[0];
            config->val = d;
        } else {
            uint8_t d = 0 | dataBuffer[0];
            config->val = d;
        }
    } else if (config->len == 2) {
        if (config->sign) {
            int16_t d = 0 | dataBuffer[0] | (dataBuffer[1] << 8);
            config->val = d;
        } else {
            uint16_t d = 0 | dataBuffer[0] | (dataBuffer[1] << 8);
            config->val = d;
        }
    } else if (config->len == 4) {
        if (config->sign) {
            int32_t d = 0 | dataBuffer[0] | (dataBuffer[1] << 8) | (dataBuffer[2] << 16) | (dataBuffer[3] << 24);
            config->val = d;
        } else {
            uint32_t d = 0 | dataBuffer[0] | (dataBuffer[1] << 8) | (dataBuffer[2] << 16) | (dataBuffer[3] << 24);
            config->val = d;
        }
    } else {
        uint32_t data = 0;
        for (int16_t i = config->len - 1; i >= 0; i--) {
            data <<= 8;
            data |= dataBuffer[i];
        }
        if (!config->sign) {
            config->val = (double) data;
        } else {
            // please dont ask about this i have know idea what i did here =)
            // seems to work, though
            // what should be happening:
            // if the data is signed and starts with a 1, convert it to int rather than uint
            uint32_t x = 0b10000000;
            x <<= (config->len - 1) * 8;
            if (data & x) {
                data ^= x;
                int32_t signeddata = (int32_t) data;
                uint32_t keep = 0x0;
                for (uint8_t i = 0; i < config->len; i++) keep = keep << 8 | 0xFF;
                signeddata &= keep;
                signeddata ^= keep;
                signeddata ^= x;
                signeddata = signeddata + 1,
                config->val = (double) -signeddata;
            } else {
                config->val = (double) data;
            }
        }
    }
    if (config->factor != 0) config->val /= config->factor;
    return true;
}

bool readToBufferUnsynchronized(char* buffer, uint16_t buffer_len, DatapointConfig *config) {
    loopOptolink();

    OptolinkTelegram sendTelegram;
    OptolinkTelegram recvTelegram;

    sendTelegram.reset();
    sendTelegram.setCmd(DATA_REQUEST);
    sendTelegram.pushData(REQUEST);
    sendTelegram.pushData(READ);
    sendTelegram.pushData(config->addr >> 8);
    sendTelegram.pushData(config->addr & 0xFF);
    sendTelegram.pushData(config->len);
    sendTelegram.writeTo(OPTOLINK_SERIAL);
    //sendTelegram.print();

    readUsefulTelegram(&recvTelegram, 500);

    if (recvTelegram.getError() != NONE) {
        switch (recvTelegram.getError()) {
            case TIMEOUT:
                snprintf(buffer, buffer_len, "TIMEOUT %02X %02X %02X", recvTelegram.getCmd(), recvTelegram.getLen(), recvTelegram.getCrc());
                return false;
            case CRC_ERROR:
                strncpy(buffer, "CRC ", buffer_len);
                snprintf(buffer + 4, buffer_len - 4, "%02X ", recvTelegram.getCalculatedCrc());
                snprintf(buffer + 7, buffer_len - 7, "%02X ", recvTelegram.getCrc());
                return false;
            default:
                snprintf(buffer, buffer_len, "UNKNOWN");
                return false;
        }
    }
    if (recvTelegram.cmd == (uint8_t) DATA_REQUEST) {
        if (recvTelegram.getData()[0] != RESPONSE ||
            recvTelegram.getData()[1] != READ) {
            snprintf(buffer, buffer_len, "RESPONSE_MISMATCH %02X %02X", recvTelegram.getData()[0], recvTelegram.getData()[1]);
            return false;
        }
        if (recvTelegram.getLen() != (5 + config->len)) {
            snprintf(buffer, buffer_len, "ANSWER_LENGTH_MISMATCH %d != %d", recvTelegram.getLen(), config->len + 5);
            return false;
        }
        if (recvTelegram.getData()[2] != (uint8_t) (config->addr >> 8) ||
            recvTelegram.getData()[3] != (uint8_t) (config->addr & 0xFF)) {
            snprintf(buffer, buffer_len, "RESPONSE_ADDRESS_MISMATCH %02X %02X != %02X %02X", recvTelegram.getData()[2], recvTelegram.getData()[3], config->addr >> 8, config->addr & 0xFF);
            return false;
        }
        if (recvTelegram.getData()[4] != config->len) {
            snprintf(buffer, buffer_len, "RESPONSE_BYTES_MISMATCH");
            return false;
        }
        if (config->hex) {
            buffer[0] = '0';
            buffer[1] = 'x';
            uint16_t offset = 2;
            for (uint8_t i = 0; i < config->len; i++) {
                uint8_t data = recvTelegram.getData()[i + 5];
                offset += snprintf(buffer + offset, buffer_len - offset, "%02X", data);
            }
            return true;
        } else {
            //factorTest(recvTelegram.getData() + 5, expectBytes, factor, sign, val);
            if (!convertData(config, recvTelegram.getData() + 5)) {
                snprintf(buffer, buffer_len, "DATA_CONVERSION_FAILED");
                return false;
            }
            print("val is (end) ");
            print(config->val);
            snprintf(buffer, buffer_len, "%.2f", config->val);
            return true;
        }
    } else {
        print("not 41");
        snprintf(buffer, buffer_len, "NOT_41 %02X %02X %02X", recvTelegram.getCmd(), recvTelegram.getLen(), recvTelegram.getCrc());
        return false;
    }
}

bool readToBuffer(char* buffer, uint16_t buffer_len, DatapointConfig *config) {
    while (link_locked) delay(1);
    link_locked = true;
    bool ret = readToBufferUnsynchronized(buffer, buffer_len, config);
    link_locked = false;
    return ret;
}

bool writeFromStringUnsynchronized(const String& value, char* buffer, uint16_t buffer_len, DatapointConfig *config) {
    bool canWrite = false;
    if (config->addr == 0x7100) canWrite = true; // kühlart
    if (config->addr == 0x7101) canWrite = true; // heizkreis
    if (config->addr == 0x7102) canWrite = true; // raumtemp soll kuehlen
    if (config->addr == 0x7103) canWrite = true; // min vorlauftemp kühlen
    if (config->addr == 0x7104) canWrite = true; // einfluss raumtemp kühlen
    if (config->addr == 0x7106) canWrite = true; // witterungs / raumtemperaturgeführte kühlung
    if (config->addr == 0x7107) canWrite = true; // welcher heizkreis / tempsensor für kühlung
    if (config->addr == 0x7110) canWrite = true; // kühlkennlinie neigung
    if (config->addr == 0x7111) canWrite = true; // kühlkennlinie steigung
    if (config->addr == 0x2001) canWrite = true; // raumtemp red soll
    if (config->addr == 0x2003) canWrite = true; // use remote control
    if (config->addr == 0x2000) canWrite = true; // raumtep soll
    if (config->addr == 0x2005) canWrite = true; // ramtemperaturregelung
    if (config->addr == 0x2006) canWrite = true; // heizkennlinie niveau
    if (config->addr == 0x2007) canWrite = true; // heizkennlinie steigung
    if (config->addr == 0x200A) canWrite = true; // Einfluss Raumtemperaturaufschaltung
    if (config->addr == 0x2034) canWrite = true; // Einfluss Raumtemperaturaufschaltung kühlen
    if (config->addr == 0xb000) canWrite = true; // betriebsmodus
    if (config->addr == 0x7002) canWrite = true; // temerpatur mittel langzeitermittlung min (counts)
    if (config->addr == 0x7003) canWrite = true; // Temperaturdifferenz heizen an = Langzeitmittel - 7003 - 2
    // Temperaturdifferenz heizen aus = Langzeitmittel - 7003 + 2
    if (config->addr == 0x7004) canWrite = true; // Temperaturdifferenz kühlgrenze Kühlgrenze = RaumSollTemp + 7004
    if (config->addr == 0x730F) canWrite = true; // Optimale Leistung bei min. Aussentemperatur
    if (config->addr == 0x7310) canWrite = true; // Optimale Leistung bei max. Aussentemperatur
    if (config->addr == 0x7414) canWrite = true; // Startleistung
    if (config->addr == 0x5006) canWrite = true; // Min. Pausenzeit Verdichter
    // 6000 WW Soll
    // B020 1x WW bereiten
    // 600C WW2 Soll
    // to start: http://192.168.11.30/write?config->addr=0xB020&len=1&val=true&stat=1
    if (config->addr == 0xB020) canWrite = true; // 1x WW bereiten
    if (config->addr == 0x6000) canWrite = true; // 1x WW Soll
    if (config->addr == 0x600C) canWrite = true; // 1x WW2 Soll

    // http://192.168.11.30/read?addr=0x1A52&conv=cop Ventilator PROZENT
    // http://192.168.11.30/read?addr=0x1A53&conv=cop Lüfter Prozent
    // http://192.168.11.30/read?addr=0x1A54&conv=cop Kompressor Prozent
    // http://192.168.11.30/read?addr=0x1AC3&conv=cop Verdichter Last Prozent
    if ((config->addr & 0xFFF0) == 0x01D0) canWrite = true;

    // kühlen langzeitermittlung: 30 min
    //                            7004: 10 //bei RaumSollTemp + 1 kühlen
    // heizen langzeitermittlung: 180min

    if (!canWrite) {
        if (buffer != nullptr) strncpy(buffer, "INVALID_ADDRESS", buffer_len);
        return false;
    }
    println("can write");

    loopOptolink();

    OptolinkTelegram sendTelegram;
    OptolinkTelegram recvTelegram;

    sendTelegram.reset();
    sendTelegram.setCmd(DATA_REQUEST);
    sendTelegram.pushData(REQUEST);
    sendTelegram.pushData(WRITE);
    sendTelegram.pushData(config->addr >> 8);
    sendTelegram.pushData(config->addr & 0xFF);
    sendTelegram.pushData(config->len);

    double d = value.toDouble();
    println(d);
    if (config->factor != 0) d = d * config->factor;
    uint32_t write = d;
    for (uint8_t i = 0; i < config->len; i++) {
        //uint8_t push = write >> (expectBytes - 1 - i) * 8 & 0xFF;
        uint8_t push = write >> (i) * 8 & 0xFF;
        sendTelegram.pushData(push);
    }

    sendTelegram.writeTo(OPTOLINK_SERIAL);
    print("send done");

    print("waiting for telegram");
    readUsefulTelegram(&recvTelegram, 500);
    print("got telegram");

    if (recvTelegram.getError() != NONE) {
        switch (recvTelegram.getError()) {
            case TIMEOUT:
                snprintf(buffer, buffer_len, "TIMEOUT %02X %02X %02X", recvTelegram.getCmd(), recvTelegram.getLen(), recvTelegram.getCrc());
                return false;
            case CRC_ERROR:
                strncpy(buffer, "CRC ", buffer_len);
                snprintf(buffer + 4, buffer_len - 4, "%02X ", recvTelegram.getCalculatedCrc());
                snprintf(buffer + 7, buffer_len - 7, "%02X ", recvTelegram.getCrc());
                return false;
                break;
            default:
                snprintf(buffer, buffer_len, "UNKNOWN");
                return false;
        }
    }
    if (recvTelegram.cmd == (uint8_t) DATA_REQUEST) {
        if (recvTelegram.getData()[0] != RESPONSE ||
            recvTelegram.getData()[1] != WRITE) {
            snprintf(buffer, buffer_len, "INV_CMD %02X %02X ", recvTelegram.getData()[0], recvTelegram.getData()[1]);
            printf("INV_CMD");
            recvTelegram.print();
            return false;
        }
        if (recvTelegram.getLen() != 5) {
            snprintf(buffer, buffer_len, "INV_LEN %d != %d", recvTelegram.getLen(), 5);
            printf("INV_LEN");
            recvTelegram.print();
            return false;
        }
        if (recvTelegram.getData()[2] != (uint8_t) (config->addr >> 8) ||
            recvTelegram.getData()[3] != (uint8_t) (config->addr & 0xFF)) {
            snprintf(buffer, buffer_len, "INV_ADDR %02X %02X != %02X %02X", recvTelegram.getData()[2], recvTelegram.getData()[3], config->addr >> 8, config->addr & 0xFF);
            printf("INV_ADDR");
            recvTelegram.print();
            return false;
        }
        if (recvTelegram.getData()[4] != config->len) {
            snprintf(buffer, buffer_len, "INV_LEN2");
            printf("INV_LEN2");
            recvTelegram.print();
            return false;
        }
        double writeVal = value.toDouble();
        if (!readToBufferUnsynchronized(buffer, buffer_len, config)) {
            strncpy(buffer, "READ_BACK_FAILED", buffer_len);
            return false;
        }
        if (config->val == writeVal) {
            return true;
        } else {
            strncpy(buffer, "READ_BACK_MISMATCH", buffer_len);
            return false;
        }
    } else {
        print("not 41");
        snprintf(buffer, buffer_len, "NOT_41 %02X %02X %02X", recvTelegram.getCmd(), recvTelegram.getLen(), recvTelegram.getCrc());
        recvTelegram.print();
        return false;
    }
}

bool writeFromString(const String& value, char* buffer, uint16_t buffer_len, DatapointConfig *config) {
    println("waiting for lock");
    while (link_locked) delay(1);
    link_locked = true;
    // when i remove println and delay writes fail?! why??? leaving it in for now..
    println("lock acquired");
    delay(10);
    bool ret = writeFromStringUnsynchronized(value, buffer, buffer_len, config);
    link_locked = false;
    return ret;
}
