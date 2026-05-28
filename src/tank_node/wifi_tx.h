#ifndef WIFI_TX_H
#define WIFI_TX_H

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

void sendTankData(void);
void handleValveCommandOpen(void);
void handleValveCommandClose(void);


#endif