#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#include "HTML/web_server.h"

// Static Functions
static void VoltAmps_Updater(void);
static void MotorButtonCylicCheck(void);

void sendValveCommand(bool valveOpen);

//G variables

unsigned long MotorStartButton_Time =0;
unsigned long MotorSTopButton_Time =0;

// DIO
//PZEM004t D5 D6 are used

// RX, TX
SoftwareSerial pzemSWSerial(D5, D6);

// Create PZEM object
PZEM004Tv30 pzem(pzemSWSerial);


const char* WIFI_SSID = "DIGISOL";
const char* WIFI_PASSWORD = "Srn12345";

#define RELAYSTART D1
#define RELAYSTOP D2

void setup()
{
    pinMode(RELAYSTART,OUTPUT);
    digitalWrite(RELAYSTART,LOW);
    pinMode(RELAYSTOP,OUTPUT);
    digitalWrite(RELAYSTOP,LOW);
    delay(2000);
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
    VoltAmps_Updater();

    MotorButtonCylicCheck();

    webServerHandleClient();




    delay(10);
}

static void VoltAmps_Updater(void)
{
    float voltage = pzem.voltage();
    float current = pzem.current();

    if (!isnan(voltage))
    {
        g_supplyVoltage = voltage;
    }

    if(!isnan(current))
    {
        g_supplyCurrent = current;
    }

}

u8 g_metturValveStatus_last =255;

static void MotorButtonCylicCheck(void)
{
    if((MotorStartButton_Time != 0u)&&(millis() - MotorStartButton_Time)> 1500u)
    {
       digitalWrite(RELAYSTART,LOW);
       Serial.println("Bore Motor START over");
       MotorStartButton_Time = 0u;
    }

    
    if((MotorSTopButton_Time != 0u)&&(millis() - MotorSTopButton_Time)> 500u)
    {
       digitalWrite(RELAYSTOP,LOW);
       MotorSTopButton_Time = 0u;
    }

    if((g_metturValveStatus == 1u) && (g_metturValveStatusTime!=0u)&&((millis() - g_metturValveStatusTime) > 20000u ))
    {
        g_metturValveStatus = 3u;
        g_metturValveStatusTime =0;
    }
    if((g_metturValveStatus == 2u) && (g_metturValveStatusTime!=0u)&&((millis() - g_metturValveStatusTime) > 20000u ))
    {
        g_metturValveStatus = 4u;
        g_metturValveStatusTime =0;
    }
    if(g_metturValveStatus == 255u)
    {

        if(g_metturValveStatusTime!=0u)
        {
            sendValveCommand(false);
            g_metturValveStatusTime = millis();
        }
        if((millis() - g_metturValveStatusTime)>20000u)
        {
            g_metturValveStatus = 4u;
            g_metturValveStatusTime =0;
            Serial.println("test2");
        }
    }
    if((g_metturValveStatus == 1)&&(g_metturValveStatus_last!=g_metturValveStatus))
    {
        sendValveCommand(true);
        g_metturValveStatus_last = g_metturValveStatus;
    }
    if((g_metturValveStatus == 2)&&(g_metturValveStatus_last!=g_metturValveStatus))
    {
        sendValveCommand(false);
        g_metturValveStatus_last = g_metturValveStatus;
    }    


}

/* =========================================================
   Callback Functions
========================================================= */


void onBoreMotorStart(void)
{
    digitalWrite(RELAYSTART,HIGH);
    MotorStartButton_Time = millis();
    Serial.println("Bore Motor START");
}

void onBoreMotorStop(void)
{
    digitalWrite(RELAYSTOP,HIGH);
    MotorSTopButton_Time = millis();
    Serial.println("Bore Motor STOP");
}

void onMetturValveOpen(void)
{
    //sendValveCommand(true);
}
void onMetturValveClose(void)
{
    //sendValveCommand(false);
}

/* =========================================================
   Send Data To Tank Node
========================================================= */

void sendValveCommand(bool valveOpen)
{
    HTTPClient http;

    WiFiClient client;

    String url;


    if (valveOpen == true)
    {
       url = "http://192.168.2.51/valve/true";
    }
    else
    {
        url = "http://192.168.2.51/valve/false";
    }

    http.begin(client, url);
    http.GET();

    http.end();
}