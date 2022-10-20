#include <SoftwareSerial.h>

#include "../include/Co2cm11.h"


Co2cm11::Co2cm11(int co2rxPin, int co2txPin) {
    Serial.println("CO2 Create");
    co2Serial = new SoftwareSerial(co2rxPin, co2txPin);
}

void Co2cm11::init(bool debugEnabled) {
    debug = debugEnabled;
    co2Serial->begin(9600);
    co2Serial->setTimeout(5000);
}

String Co2cm11::calibrate() {
    byte response[4];
    memset(response, 0, 4);
    sendCo2(calibrateTo400cmd, 6, response, 4);

    String res = "";
    for (int i = 0; i < 4; i++) {
        res += "0x";
        if (response[i] <= 0xE) {
            res += "0";
        }
        res += String(response[i], HEX);
        res += " ";
    }

    return res;
}

bool Co2cm11::sendCo2(byte cmd[], int cmdLen, byte response[], int recvLen) {
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
    while((avail = co2Serial->available()) < recvLen) {
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
    byte rd = co2Serial->readBytes(response, recvLen);
    bool allzero = true;
    for (int i = 0; i < recvLen; i++) {
        if(response[i] != 0x00) {
            allzero = false;
        }
    }
    if(debug) {
        Serial.print("CO2 Response: ");
        for (int i = 0; i < recvLen; i++) {
            Serial.print("0x");
            if (response[i] <= 0xE) {
                Serial.print("0");
            }
            Serial.print(response[i], HEX);
            Serial.print(" ");
        }
        Serial.println("");
    }
    if(int(rd) != recvLen) {
        Serial.println("Read " + String(rd) + ", expected: " + String(recvLen));
        return false;
    }

    if(allzero) {
        return false;
    }
    return true;
}

Co2cm11::Co2result Co2cm11::readCo2() {
    byte response[8];
    int time = millis() / 1000;
    if (co2NextCheck > time) {
        return cachedResult;
    }
    co2NextCheck = time + 1;

    memset(response, 0, 8);
    bool result = sendCo2(co2cmd, 4, response, 8);
    if (!result) {
        Serial.println("CO2 error");

        return cachedResult;
    }
    cachedResult.ppm = (256*(int)response[3]) + (int)response[4];
    cachedResult.flags = (int)response[5];

    return cachedResult;
}