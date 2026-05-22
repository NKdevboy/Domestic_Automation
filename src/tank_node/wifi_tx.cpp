#include "wifi_tx.h"

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

extern uint8_t g_borePercentage;
extern float g_boreEmptyHeight;

extern uint8_t g_metturPercentage;
extern float g_metturEmptyHeight;

extern uint8_t g_aliveCounter;

WiFiClient client;

void sendTankData(void)
{
    HTTPClient http;

    String url;

    url =
        "http://192.168.2.50/updateTankData?";

    url += "borePercent=" +
           String(g_borePercentage);

    url += "&boreEmpty=" +
           String(g_boreEmptyHeight);

    url += "&metturPercent=" +
           String(g_metturPercentage);

    url += "&metturEmpty=" +
           String(g_metturEmptyHeight);

    url += "&aliveCounter=" +
           String(g_aliveCounter);

    http.begin(client, url);

    int httpCode = http.GET();

    Serial.print("HTTP Response : ");
    Serial.println(httpCode);

    http.end();
}