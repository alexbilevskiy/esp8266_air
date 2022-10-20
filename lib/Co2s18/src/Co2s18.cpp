#include <SoftwareSerial.h>

#include "../include/Co2s18.h"


Co2s18::Co2s18(int co2rxPin, int co2txPin) {
    Serial.println("CO2 Create");
    co2Serial = new SoftwareSerial(co2rxPin, co2txPin);
}

void Co2s18::init(bool debugEnabled) {
    debug = debugEnabled;
    co2Serial->begin(9600);
    co2Serial->setTimeout(5000);
    //sendCo2(setRange, response);
    //sendCo2(setABC, response);
}

bool Co2s18::sendCo2(byte cmd[], int cmdLen, byte response[], int len) {
    if(debug) {
        Serial.print("CO2 Request: ");
        for (int i = 0; i < cmdLen; i++) {
            Serial.print("0x");
            if (cmd[i] <= 0xE) {
                Serial.print("0");
            }
            Serial.print(cmd[i], HEX);
            Serial.print(" ");
        }
        Serial.println("");
    }
    co2Serial->write(cmd, cmdLen);
    int avail = 0;
    int started = int(millis() / 1000);
    int now;
    while((avail = co2Serial->available()) < len) {
        debug && Serial.println("waiting c02 + " + String(avail));
        now = int(millis() / 1000);
        if(started < now - 10) {
            Serial.println("CO2 ERROR!!! Timeout!!!");
            Serial.println("CO2 ERROR!!! Timeout!!!");
            Serial.println("CO2 ERROR!!! Timeout!!!");
            Serial.println("CO2 ERROR!!! Timeout!!!");
            Serial.println("CO2 ERROR!!! Timeout!!!");
            break;
        }
    }
    byte rd = co2Serial->readBytes(response, len);
    bool allzero = true;
    for (int i = 0; i < len; i++) {
        if(response[i] != 0x00) {
            allzero = false;
        }
    }
    if(debug) {
        Serial.print("CO2 Response: ");
        for (int i = 0; i < len; i++) {
            Serial.print("0x");
            if (response[i] <= 0xE) {
                Serial.print("0");
            }
            Serial.print(response[i], HEX);
            Serial.print(" ");
        }
        Serial.println("");
    }
    if(int(rd) != len) {
        Serial.println("Read " + String(rd) + ", expected: " + String(len));
        return false;
    }

    if(allzero) {
        return false;
    }
    return true;
}

Co2s18::Co2result Co2s18::readCo2() {
    byte response[13];
    int time = millis() / 1000;
    if (co2NextCheck > time) {
        return cachedResult;
    }
    co2NextCheck = time + 1;

    memset(response, 0, 13);
    bool result = sendCo2(co2cmd, 8, response, 13);
    if (!result) {
        Serial.println("CO2 error");

        return cachedResult;
    }
    cachedResult.s1 = (256*(int)response[3]) + (int)response[4];
    cachedResult.s2 = (256*(int)response[5]) + (int)response[6];
    cachedResult.s3 = (256*(int)response[7]) + (int)response[8];
    cachedResult.ppm = (256*(int)response[9]) + (int)response[10];
    cachedResult.temp = (256*(int)response[11]) + (int)response[12];
    cachedResult.abc = readOne(abcCmd);
    cachedResult.cal = readOne(calCmd);

    return cachedResult;
}

String Co2s18::readOne(byte *cmd) {
    byte response[7];
    memset(response, 0, 7);
    bool result = sendCo2(cmd, 8, response, 7);
    if (!result) {
        return "0";
    }
    int one = (256*(int)response[3]) + (int)response[4];
    return String(one);
}
