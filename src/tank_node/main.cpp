#include <Arduino.h>

#include <ESP8266WiFi.h>

#include "ultrasonic.h"
#include "tank_logic.h"
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

/* =====================================================
   Tank Heights
===================================================== */

#define BORE_TANK_HEIGHT_CM      162.56f

#define METTUR_TANK_HEIGHT_CM    127.0f

/* =====================================================
   Global Tank Data
===================================================== */

uint8_t g_borePercentage = 0;

float g_boreEmptyHeight = 0;

uint8_t g_metturPercentage = 0;

float g_metturEmptyHeight = 0;

/* =====================================================
   Alive Counter
===================================================== */

uint8_t g_aliveCounter = 0;

/* =====================================================
   Local Structures
===================================================== */

TankData_t boreTank;

TankData_t metturTank;

/* =====================================================
   Average Read Function
===================================================== */

float ultrasonicAverageRead(uint8_t trigPin,
                            uint8_t echoPin)
{
    float total = 0;

    for (uint8_t i = 0; i < 20; i++)
    {
        total +=
            ultrasonicReadCm(trigPin,
                              echoPin);

        delay(50);
    }

    return (total / 20.0f);
}

/* =====================================================
   WiFi Connect
===================================================== */

void wifiConnect(void)
{
    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID,
               WIFI_PASSWORD);

    Serial.print("Connecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);

        Serial.print(".");
    }

    Serial.println();

    Serial.println("WiFi Connected");

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

    server.on("/valve/true",
          handleValveCommandOpen);

    server.on("/valve/false",
          handleValveCommandClose);

    server.begin();
    digitalWrite(METTUR_VALVE_PIN,LOW);
}

/* =====================================================
   Loop
===================================================== */

void loop()
{
    float boreDistance;

    float metturDistance;

    /* =================================================
       Bore Tank
    ================================================= */

    boreDistance =
        ultrasonicAverageRead(
            BORE_TRIG_PIN,
            BORE_ECHO_PIN);

    tankCalculate(
        BORE_TANK_HEIGHT_CM,
        boreDistance,
        &boreTank);

    g_borePercentage =
        boreTank.percentage;

    g_boreEmptyHeight =
        boreTank.emptyHeightCm;

    /* =================================================
       Mettur Tank
    ================================================= */

    metturDistance =
        ultrasonicAverageRead(
            METTUR_TRIG_PIN,
            METTUR_ECHO_PIN);

    tankCalculate(
        METTUR_TANK_HEIGHT_CM,
        metturDistance,
        &metturTank);

    g_metturPercentage =
        metturTank.percentage;

    g_metturEmptyHeight =
        metturTank.emptyHeightCm;

    /* =================================================
       Alive Counter Ring Logic
    ================================================= */

    g_aliveCounter =
        (g_aliveCounter + 1) % 15;

    /* =================================================
       Send Data
    ================================================= */
    server.handleClient();
    sendTankData();

    delay(10);
}

void handleValveCommandOpen(void)
{
    digitalWrite(METTUR_VALVE_PIN,HIGH);
    Serial.println("Value open");
}
void handleValveCommandClose(void)
{
    digitalWrite(METTUR_VALVE_PIN,LOW);
    Serial.println("Value close");
}