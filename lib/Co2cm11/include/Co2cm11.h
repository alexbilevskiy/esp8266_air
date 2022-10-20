#include <SoftwareSerial.h>

#ifndef ESP8266_AIR_CO2_CM11_H
#define ESP8266_AIR_CO2_CM11_H

class Co2cm11 {
public:
    bool debug = false;

    struct Co2result {
//        int s1;
//        int s2;
//        int s3;
        int ppm;
        int flags;
//        int temp;
//        String abc;
//        String cal;
    };

    Co2cm11(int co2rxPin, int co2txPin);
    void init(bool debugEnabled);

    String calibrate();

    Co2result readCo2();

private:
    SoftwareSerial* co2Serial;

    byte co2cmd[4] = {0x11, 0x01, 0x01, 0xED};
    byte calibrateTo400cmd[6] = {0x11, 0x03, 0x03, 0x01, 0x90, 0x58};

    int co2NextCheck = 0;

    bool sendCo2(byte *cmd, int cmdLen, byte *response, int recvLen);

    Co2result cachedResult;
};


#endif //ESP8266_AIR_CO2_CM11_H
