#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "../lib/Co2s18/include/Co2s18.h"
#include "../lib/Co2cm11/include/Co2cm11.h"
#include "config.h"

bool debug = false;

WiFiServer server(80);
WiFiClient wifiClient;
WiFiClient mqttWifiClient;
PubSubClient mqttClient(mqttWifiClient);
Co2s18 co2s18(co2rxPin, co2txPin);
Co2cm11 co2cm11(cm11rxPin, cm11txPin);

void respond(const String& text) {
    wifiClient.println("HTTP/1.1 200 OK");
    wifiClient.println("Content-Type: text/html");
    wifiClient.println("Connection: close");
    wifiClient.println("");
    wifiClient.println(text);
    wifiClient.flush();
}

String payload;
char payloadBytes[150];
String uptime;
Co2cm11::Co2result cm11res;
Co2s18::Co2result s18res;
String req;

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    server.begin();
    delay(500);
    WiFi.begin(wifi_ssid, wifi_password);
    delay(10000);

    if(s18enabled) {
        co2s18.init(debug);
    }
    if(cm11enabled) {
        co2cm11.init(debug);
    }
    if(mqttEnabled) {
        mqttClient.setServer(mqtt_server, mqtt_port);
    }
}

void loop() {
    mqttEnabled && mqttClient.loop();

    int s = WiFi.status();
    if (s == WL_DISCONNECTED || s == WL_IDLE_STATUS || s == WL_CONNECT_FAILED || s == WL_CONNECTION_LOST) {
        Serial.println("Disconnected, reconnecting!");
        WiFi.disconnect();
        ESP.restart();

        WiFi.mode(WIFI_STA);
        WiFi.begin(wifi_ssid, wifi_password);
        delay(10000);
        return;
    }
    if (s != WL_CONNECTED) {
        Serial.println("not connected: " + String(s));
        delay(1000);
        return;
    }

    if (mqttEnabled && !mqttClient.connected()) {
        Serial.println("Disconnected mqtt, reconnecting!");
        mqttClient.disconnect();
        mqttClient.connect(mqttClientId);
        return;
    }
    mqttEnabled && mqttClient.loop();

    uptime = String(int(millis() / 1000));
    if(s18enabled) {
        debug && Serial.println("read co2 s18");
        s18res = co2s18.readCo2();
    }
    if(cm11enabled) {
        debug && Serial.println("read co2 cm11");
        cm11res = co2cm11.readCo2();
    }


    payload = "{";
    if(s18enabled) {
        payload +=
                "\"co2_s1\":" + String(s18res.s1) + "," +
                "\"co2_s2\":" + String(s18res.s2) + "," +
                "\"co2_s3\":" + String(s18res.s3) + "," +
                "\"co2_ppm_s8\":" + String(s18res.ppm) + "," +
                "\"co2_abc\":" + String(s18res.abc) + "," +
                "\"co2_cal\":" + String(s18res.cal) + ",";
    }
    if(cm11enabled) {
        payload +=
            "\"co2_ppm_cm11\":" + String(cm11res.ppm) + "," +
            "\"co2_flags\":" + String(cm11res.flags) + ",";
    }
    payload +=
            "\"uptime\":" + uptime +
        "}";

    if(debug) {
        Serial.println(payload);
    } else {
        Serial.print(".");
    }
    payload.toCharArray(payloadBytes, 200);
    mqttEnabled && mqttClient.publish(mqttTopic, payloadBytes);
    mqttEnabled && mqttClient.loop();

    wifiClient = server.available();
    if (!wifiClient) {
        debug && Serial.println("no client...");
        delay(500);

        return;
    }
    req = wifiClient.readStringUntil('\r');
    Serial.println("request: " + req);
    if (req.indexOf("/metrics") >= 0) {
        respond(payload);

        return;
    }
    if (req.indexOf("/cal") >= 0) {
        if(cm11enabled) {
            co2cm11.debug = true;
            String cres = co2cm11.calibrate();
            co2cm11.debug = debug;
            respond(cres);
        } else {
            respond("Not supported");
        }

        return;
    }

    if (req.indexOf("/debug") >= 0) {
        debug = !debug;
        if(s18enabled) {
            co2s18.debug = debug;
        }
        if(cm11enabled) {
            co2cm11.debug = debug;
        }
        respond(String(debug));

        return;
    }

    respond("My MAC: " + WiFi.macAddress() + ", my IP: " + WiFi.localIP().toString());
}

