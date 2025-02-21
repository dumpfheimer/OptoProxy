//
// Created by chris on 7/10/23.
//
#include "optolink.h"

bool link_locked = false;
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
void readTelegram(OptolinkTelegram* telegram) {
    println("readTelegram");
    if (telegram == nullptr) return;
    telegram->reset();

    uint8_t charBuffer;
    if (!read(OPTOLINK_SERIAL, &charBuffer, 500)) {
        telegram->setError(TIMEOUT);
        return;
    }
    telegram->setCmd(charBuffer);
    if (telegram->getCmd() == 0x05) return;
    if (telegram->getCmd() == 0x06) return;
    if (telegram->getCmd() == 0x15) return;
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

void readUsefulTelegram(OptolinkTelegram* telegram) {
    readTelegram(telegram);
    println(telegram->getCmd());
    /*if (currentTelegram.getCmd() == 0x05) {
        // ping
        currentTelegram.reset();
        currentTelegram.setCmd(0x04);
        currentTelegram.writeTo(OPTOLINK_SERIAL);
        return readUsefulTelegram();
    }*/
    if (telegram->getCmd() == 0x05) {
        println("got ping msg");
        // ping
        OPTOLINK_SERIAL.write(0x04);
        readTelegram(telegram);
        if (telegram->getCmd() == 0x05) {
            println("got second ping");
            telegram->reset();
            telegram->setCmd(0x16);
            telegram->writeTo(OPTOLINK_SERIAL);
            readTelegram(telegram);
            if (telegram->getCmd() == 0x06) {
                return readUsefulTelegram(telegram);
            }
        } else {
            println("did not get second ping message");
        }
    }
    if (telegram->getCmd() == 0x06) {
        // ack
        return readUsefulTelegram(telegram);
    }
}

bool loopOptolink() {
    if (!OPTOLINK_SERIAL.available()) return false;
    OptolinkTelegram loopTelegram;
    OPTOLINK_SERIAL.setTimeout(50);
    readTelegram(&loopTelegram);
    println("read a telegram in loop");
    print(loopTelegram.getCmd());
    if (loopTelegram.getCmd() == 0x05) {
        while (OPTOLINK_SERIAL.available()) OPTOLINK_SERIAL.read();
        OPTOLINK_SERIAL.write(0x04);
        println("waiting for second telegram");
        waitAvailable(OPTOLINK_SERIAL, 2000);
        readTelegram(&loopTelegram);
        println("read second telegram");
        if (loopTelegram.getCmd() == 0x05) {
            println("was 05 too");
            while (OPTOLINK_SERIAL.available()) OPTOLINK_SERIAL.read();
            loopTelegram.reset();
            loopTelegram.setCmd(0x16);
            loopTelegram.writeTo(OPTOLINK_SERIAL);
            readTelegram(&loopTelegram);
            println("read third telegram");
            if (loopTelegram.getCmd() == 0x06) {
                println("was 06, handshake completed");
                while (OPTOLINK_SERIAL.available()) OPTOLINK_SERIAL.read();
                return true;
            }
        }
    }
    return false;
}

bool convertData(DatapointConfig *config, uint8_t dataBuffer[]) {
    print("cD factor");
    println(config->factor);
    if (config->factor == 0) config->factor += 1;
    uint32_t data = 0;
    for (int16_t i = config->len - 1; i >= 0; i--) {
        data <<= 8;
        data |= dataBuffer[i];
    }
    double val = 0;
    if (!config->sign) {
        val = (double) data;
    } else {
        uint32_t x = 0b10000000;
        x <<= (config->len - 1) * 8;
        if (data & x) {
            int32_t signeddata = (int32_t) -data;
            uint32_t keep = 0x0;
            for (uint8_t i = 0; i < config->len; i++) keep = keep << 8 | 0xFF;
            signeddata &= keep;
            signeddata = signeddata + 1,
            val = (double) -signeddata;
        } else {
            val = (double) data;
        }
    }
    print("cD factor");
    println(config->factor);
    print("val is ");
    print(val);
    print(" factor is ");
    print(config->factor);
    if (config->factor != 0) val /= config->factor;
    config->val = val;
    return true;
}

bool readToBufferUnsynchronized(char* buffer, DatapointConfig *config) {
    print(" factor ");
    print(config->factor);
    loopOptolink();

    OptolinkTelegram sendTelegram;
    OptolinkTelegram recvTelegram;

    sendTelegram.reset();
    sendTelegram.setCmd(0x41);
    sendTelegram.pushData(0x00);
    sendTelegram.pushData(0x01);
    sendTelegram.pushData(config->addr >> 8);
    sendTelegram.pushData(config->addr & 0xFF);
    sendTelegram.pushData(config->len);
    sendTelegram.writeTo(OPTOLINK_SERIAL);
    //sendTelegram.print();

    readUsefulTelegram(&recvTelegram);

    if (recvTelegram.getError() != NONE) {
        switch (recvTelegram.getError()) {
            case TIMEOUT:
                sprintf(buffer, "TIMEOUT %02X %02X %02X", recvTelegram.getCmd(), recvTelegram.getLen(), recvTelegram.getCrc());
                return false;
            case CRC_ERROR:
                strcpy(buffer, "CRC ");
                sprintf(buffer + 4, "%02X ", recvTelegram.getCalculatedCrc());
                sprintf(buffer + 7, "%02X ", recvTelegram.getCrc());
                //strcpy(buffer + 10, recvTelegram.toString().c_str());
                return false;
                break;
            default:
                sprintf(buffer, "UNKNOWN");
                return false;
        }
    }
    if (recvTelegram.cmd == (uint8_t) 0x41) {
        if (recvTelegram.getData()[0] != 0x01 ||
            recvTelegram.getData()[1] != 0x01) {
            sprintf(buffer, "RESPONSE_MISMATCH %02X %02X", recvTelegram.getData()[0], recvTelegram.getData()[1]);
            return false;
        }
        if (recvTelegram.getLen() != (5 + config->len)) {
            sprintf(buffer, "ANSWER_LENGTH_MISMATCH %d != %d", recvTelegram.getLen(), config->len + 5);
            return false;
        }
        if (recvTelegram.getData()[2] != (uint8_t) (config->addr >> 8) ||
            recvTelegram.getData()[3] != (uint8_t) (config->addr & 0xFF)) {
            sprintf(buffer, "RESPONSE_ADDRESS_MISMATCH %02X %02X != %02X %02X", recvTelegram.getData()[2], recvTelegram.getData()[3], config->addr >> 8, config->addr & 0xFF);
            return false;
        }
        if (recvTelegram.getData()[4] != config->len) {
            sprintf(buffer, "RESPONSE_BYTES_MISMATCH");
            return false;
        }
        print(" factor2 ");
        println(config->factor);
        //factorTest(recvTelegram.getData() + 5, expectBytes, factor, sign, val);
        if (!convertData(config, recvTelegram.getData() + 5)) {
            sprintf(buffer, "DATA_CONVERSION_FAILED");
            return false;
        }
        print(" factor3 ");
        println(config->factor);
        print("val is (end) ");
        print(config->val);
        delay(100);
        sprintf(buffer, "%.2f", config->val);
        return true;
    } else {
        print("not 41");
        sprintf(buffer, "NOT_41 %02X %02X %02X", recvTelegram.getCmd(), recvTelegram.getLen(), recvTelegram.getCrc());
        return false;
    }
}

bool readToBuffer(char* buffer, DatapointConfig *config) {
    while (link_locked) delay(1);
    link_locked = true;
    bool ret = readToBufferUnsynchronized(buffer, config);
    link_locked = false;
    return ret;
}

bool writeFromStringUnsynchronized(const String& value, char* buffer, DatapointConfig *config) {
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
        if (buffer != nullptr) strcpy(buffer, "INVALID_ADDRESS");
        return false;
    }

    print("loop start");
    print(" factor ");
    print(config->factor);
    loopOptolink();
    print("loop end");

    OptolinkTelegram sendTelegram;
    OptolinkTelegram recvTelegram;

    sendTelegram.reset();
    sendTelegram.setCmd(0x41);
    sendTelegram.pushData(0x00);
    sendTelegram.pushData(0x02);
    sendTelegram.pushData(config->addr >> 8);
    sendTelegram.pushData(config->addr & 0xFF);
    sendTelegram.pushData(config->len);

    double d = value.toDouble();
    print("write double is ");
    println(d);
    if (config->factor != 0) d = d * config->factor;
    uint32_t write = d;
    print("write is ");
    print(write);
    print("expect bytes");
    print(config->len);
    for (uint8_t i = 0; i < config->len; i++) {
        //uint8_t push = write >> (expectBytes - 1 - i) * 8 & 0xFF;
        uint8_t push = write >> (i) * 8 & 0xFF;
        print("push ");
        println(push);
        sendTelegram.pushData(push);
    }

    sendTelegram.writeTo(OPTOLINK_SERIAL);
    print("send done");
    //sendTelegram.print();

    print("waiting for telegram");
    readUsefulTelegram(&recvTelegram);
    print("got telegram");

    if (recvTelegram.getError() != NONE) {
        switch (recvTelegram.getError()) {
            case TIMEOUT:
                sprintf(buffer, "TIMEOUT %02X %02X %02X", recvTelegram.getCmd(), recvTelegram.getLen(), recvTelegram.getCrc());
                return false;
            case CRC_ERROR:
                strcpy(buffer, "CRC ");
                sprintf(buffer + 4, "%02X ", recvTelegram.getCalculatedCrc());
                sprintf(buffer + 7, "%02X ", recvTelegram.getCrc());
                //strcpy(buffer + 10, recvTelegram.toString().c_str());
                return false;
                break;
            default:
                sprintf(buffer, "UNKNOWN");
                return false;
        }
    }
    if (recvTelegram.cmd == (uint8_t) 0x41) {
        if (recvTelegram.getData()[0] != 0x01 ||
            recvTelegram.getData()[1] != 0x01) {
            sprintf(buffer, "RESPONSE_MISMATCH %02X %02X", recvTelegram.getData()[0], recvTelegram.getData()[1]);
            return false;
        }
        if (recvTelegram.getLen() != (5 + config->len)) {
            sprintf(buffer, "ANSWER_LENGTH_MISMATCH %d != %d", recvTelegram.getLen(), config->len + 5);
            return false;
        }
        if (recvTelegram.getData()[2] != (uint8_t) (config->addr >> 8) ||
            recvTelegram.getData()[3] != (uint8_t) (config->addr & 0xFF)) {
            sprintf(buffer, "RESPONSE_ADDRESS_MISMATCH %02X %02X != %02X %02X", recvTelegram.getData()[2], recvTelegram.getData()[3], config->addr >> 8, config->addr & 0xFF);
            return false;
        }
        if (recvTelegram.getData()[4] != config->len) {
            sprintf(buffer, "RESPONSE_BYTES_MISMATCH");
            return false;
        }
        print("converting data");
        if (!convertData(config, recvTelegram.getData() + 5)) {
            sprintf(buffer, "DATA_CONVERSION_FAILED");
            return false;
        }
        sprintf(buffer, "%.2f", config->val);
        print("val is (end) ");
        print(config->val);
        return true;
    } else {
        print("not 41");
        sprintf(buffer, "NOT_41 %02X %02X %02X", recvTelegram.getCmd(), recvTelegram.getLen(), recvTelegram.getCrc());
        return false;
    }
}

bool writeFromString(const String& value, char* buffer, DatapointConfig *config) {
    while (link_locked) delay(1);
    link_locked = true;
    bool ret = writeFromStringUnsynchronized(value, buffer, config);
    link_locked = false;
    return ret;
}
