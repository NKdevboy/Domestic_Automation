#include <Arduino.h>

#include <ESP8266WiFi.h>

#include "ultrasonic.h"
#include "wifi_tx.h"

/* =====================================================
   WiFi Credentials
===================================================== */

const char* WIFI_SSID = "DIGISOL";
const char* WIFI_PASSWORD = "Srn12345";

/* =====================================================
   HC-SR04 Pins
===================================================== */

#define BORE_TRIG_PIN     D1
#define BORE_ECHO_PIN     D2

#define METTUR_TRIG_PIN   D5
#define METTUR_ECHO_PIN   D6

//#define METTUR_VALVE_PIN   LED_BUILTIN
#define METTUR_VALVE_PIN   D7

#define MVSTOP 0u
#define MVSTART 1u
#define MVRETRY 3u

/* =====================================================
   Global Tank Data
===================================================== */

float g_boreEmptyHeight = 0;

float g_metturEmptyHeight = 0;


unsigned long SendTankDataTimer =0;
unsigned long ultrasonicTimer =0;

uint16 ultrasonicSampleData =0u;

uint8 g_valveStatus_u8 = MVSTOP;


/* =====================================================
   WiFi Connect
===================================================== */

void wifiConnect(void)
{
    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID,WIFI_PASSWORD);

    WiFi.setAutoReconnect(true);

    WiFi.persistent(true);

    WiFi.setSleep(false);

    Serial.println();

    Serial.print("Connecting");


    while(WiFi.status()!=WL_CONNECTED){

        delay(500);
        Serial.print(".");
    }

    Serial.println();

    Serial.println("Connected");

    Serial.print("IP Address : ");

    Serial.println(WiFi.localIP());

}

/* =====================================================
   Setup
===================================================== */
ESP8266WebServer server(80);


void setup()
{
    Serial.begin(9600);

    pinMode(BORE_TRIG_PIN, OUTPUT);
    pinMode(BORE_ECHO_PIN, INPUT);

    pinMode(METTUR_TRIG_PIN, OUTPUT);
    pinMode(METTUR_ECHO_PIN, INPUT);

    pinMode(METTUR_VALVE_PIN, OUTPUT);
    
    wifiConnect();

    server.on("/v/1",
          handleValveCommandOpen);

    server.on("/v/0",
          handleValveCommandClose);

    server.begin();
    digitalWrite(METTUR_VALVE_PIN,LOW);
}

/* =====================================================
   Loop
===================================================== */

void loop()
{

    if((ultrasonicTimer==0)||((millis()-ultrasonicTimer)>200))
    {

    /* =================================================
       Bore Tank
    ================================================= */

        g_boreEmptyHeight += ultrasonicReadCm(BORE_TRIG_PIN,BORE_ECHO_PIN);

    /* =================================================
       Mettur Tank
    ================================================= */

        g_metturEmptyHeight += ultrasonicReadCm(METTUR_TRIG_PIN,METTUR_ECHO_PIN);
    
        ultrasonicSampleData++;
        ultrasonicTimer = millis();

    }

    /* =================================================
       Send Data
    ================================================= */
    server.handleClient();

    if((SendTankDataTimer == 0)||((millis()-SendTankDataTimer)>2000))
    {
        g_boreEmptyHeight /= ultrasonicSampleData;
        g_metturEmptyHeight /= ultrasonicSampleData;

        sendTankData();

        ultrasonicSampleData =0;
        g_boreEmptyHeight =0;
        g_metturEmptyHeight=0;

        SendTankDataTimer = millis();
    }

    yield();
}

void handleValveCommandOpen(void)
{
    digitalWrite(METTUR_VALVE_PIN,HIGH);
    g_valveStatus_u8 = MVSTART;
    Serial.println("Value open");
}

void handleValveCommandClose(void)
{
    digitalWrite(METTUR_VALVE_PIN,LOW);
    g_valveStatus_u8 = MVSTOP;
    Serial.println("Value close");
}