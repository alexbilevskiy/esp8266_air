#ifndef ESP8266_AIR_CONFIG_H
#define ESP8266_AIR_CONFIG_H

#define wifi_ssid "ssid"
#define wifi_password "password"
#define mqtt_server "192.168.1.1"
#define mqtt_port 1883

#define co2txPin D1
#define co2rxPin D2
#define cm11txPin D6
#define cm11rxPin D7

#define mqttTopic "wifi2mqtt/ESP_air_02"
#define mqttClientId "ESP_air_02"
#define mqttEnabled true
#define s18enabled true
#define cm11enabled true

#endif //ESP8266_AIR_CONFIG_H
