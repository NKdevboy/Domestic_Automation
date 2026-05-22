#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "HTML/web_server.h"

const char* WIFI_SSID = "DIGISOL";
const char* WIFI_PASSWORD = "Srn12345";

void setup()
{
    Serial.begin(9600);

    Serial.println();
    Serial.println("Connecting to WiFi...");
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    /* Wait until connected */
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi Connected!");
    
    /* Print IP Address */
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    webServerInit();
}


void loop()
{
    webServerHandleClient();

    /* =====================================================
       Example Dynamic Data Update
    ===================================================== */

    //g_supplyVoltage ++;

    //g_supplyCurrent ++;

    //g_boreTankLevel ++;

    //g_metturTankLevel ++;

    delay(10);
}
