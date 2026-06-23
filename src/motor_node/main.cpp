#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h>

#include "Html.h"

#define SERIAL_BEGIN

const char* ssid     = "DIGISOL";
const char* password = "Srn12345";

const char* valveNodeIP = "192.168.2.51";

#define BORE_TANK_HEIGHT_CM      162.56f
#define METTUR_TANK_HEIGHT_CM    127.0f

#define BMSTOP 0u
#define BMSTART 1u

#define MVCLOSE 0u
#define MVOPEN 1u

#define MVSTOP 0u
#define MVSTART 1u
#define MVRETRY 3u



ESP8266WebServer server(80);

//--------------------------------------------------
// SAMPLE VARIABLES
//--------------------------------------------------

float g_voltage = 0.0;
float g_current = 0.0;

uint8 g_motorStatus = BMSTOP;
uint8 g_valveStatus_u8 = MVSTOP;


float g_boreTank = 0u;
float g_boreEmpty = 0.0;

float g_metturTank = 0u;
float g_metturEmpty = 0.0;

uint8 g_s1Alive = 15u;
uint8 g_s2Alive = 15u;

//--------------------------------------------------
// AUTO OFF VARIABLES
//--------------------------------------------------

bool g_autoOffEnabled = false;

uint8 g_autoOffMinutes = 0u;

unsigned long g_autoOffStartMillis = 0u;

unsigned long remainingTime_i =0;


// DIO
//PZEM004t D5 D6 are used

// RX, TX
SoftwareSerial pzemSWSerial(D5, D6);

// Create PZEM object
PZEM004Tv30 pzem(pzemSWSerial);


//
// motor starter Digital pins
//

#define RELAYSTART D1
#define RELAYSTOP D2


//
//
//

String remainingTime_s = "--m--s";

//
// Value Operation
//

void sendValveCommand(bool valveOpen)
{
    HTTPClient http;

    WiFiClient client;

    String url;

    if(valveOpen == true)
    {
        url = String("http://") + valveNodeIP + "/v/1";
    }
    else
    {
        url = String("http://") + valveNodeIP + "/v/0";
    }

    Serial.print("Sending : ");
    Serial.println(url);

    http.begin(client, url);

    http.setTimeout(1000);

    int httpCode = http.GET();

    if(httpCode<0)
    {
        g_valveStatus_u8 = MVRETRY;
    }

    http.end();
}


//-----------------------------------------------------------------------
// Tank Percentage
//-----------------------------------------------------------------------

float tankCalculate(float tankHeightCm,float sensorDistanceCm)
{
    float waterHeight;

    float percentage;

    if (sensorDistanceCm > tankHeightCm)
    {
        sensorDistanceCm = tankHeightCm;
    }

    waterHeight =
        tankHeightCm - sensorDistanceCm;

    percentage =
        (waterHeight / tankHeightCm) * 100.0f;

    if (percentage > 100.0f)
    {
        percentage = 100.0f;
    }

    if (percentage < 0.0f)
    {
        percentage = 0.0f;
    }

    return percentage;
}

//--------------------------------------------------
// ROOT PAGE
//--------------------------------------------------

void handleRoot(){

    server.sendHeader("Cache-Control", "max-age=86400");
    
    server.send_P(
        200,
        "text/html",
        webpage
    );
}

//--------------------------------------------------
// JSON DATA
//--------------------------------------------------

    uint8 g_s2Alive_last =255u;
    uint8 g_s2Alive_lostCnt =0u;

void handleGetData(){

    String autoStatus =
        g_autoOffEnabled
        ? "ENABLED"
        : "DISABLED";

    String S_motorStatus = "STOPPED";
    String l_valveStatus = "CLOSED";

    if(g_valveStatus_u8 == MVRETRY)
    {
       // l_valveStatus = "R_SNA";
    }
    else if(g_valveStatus_u8 == MVSTOP)
    {
        l_valveStatus = "CLOSED";
    }
    else if(g_valveStatus_u8 == MVSTART)
    {
        l_valveStatus = "OPENED";
    }

    if(g_motorStatus == BMSTOP)
    {
        S_motorStatus = "STOPPED";
    }
    else if(g_motorStatus == BMSTART)
    {
        S_motorStatus = "STARTED";
    }


    if(g_s2Alive_last == g_s2Alive)
    {
        g_s2Alive_lostCnt++;
        if(g_s2Alive_lostCnt >=5u)
        {
            g_boreEmpty = 0.0;
            g_boreTank =0.0;
            g_metturTank = 0.0;
            g_metturEmpty =0.0;
            l_valveStatus = "SNA";

        }
    }
    else
    {
        g_s2Alive_lostCnt =0;
    }

    g_s2Alive_last = g_s2Alive;

    unsigned long totalSeconds =
        remainingTime_i / 1000;

    unsigned long mins =
        totalSeconds / 60;

    unsigned long secs =
        totalSeconds % 60;

    remainingTime_s =
        String(mins) + " m " +
        String(secs) + " s";


    String json = "{";

    json += "\"s1Alive\":";
    json += String(g_s1Alive);
    json += ",";

    json += "\"s2Alive\":";
    json += String(g_s2Alive);
    json += ",";

    json += "\"voltage\":";
    json += String(g_voltage,2);
    json += ",";

    json += "\"current\":";
    json += String(g_current,2);
    json += ",";

    json += "\"motorStatus\":\"";
    json += S_motorStatus;
    json += "\",";

    json += "\"valveStatus\":\"";
    json += l_valveStatus;
    json += "\",";

    json += "\"boreTank\":";
    json += String(g_boreTank);
    json += ",";

    json += "\"boreEmpty\":";
    json += String(g_boreEmpty);
    json += ",";

    json += "\"metturTank\":";
    json += String(g_metturTank);
    json += ",";

    json += "\"metturEmpty\":";
    json += String(g_metturEmpty);
    json += ",";

    json += "\"autoStatus\":\"";
    json += autoStatus;
    json += "\",";

    json += "\"remainingTime\":\"";
    json += remainingTime_s;
    json += "\"";

    json += "}";

    server.send(
        200,
        "application/json",
        json
    );

    g_s1Alive = (g_s1Alive+1u)%15u;
}

unsigned long BmotorButton_Time =0;

//
// motor Start
//

void BMotorStartOperation(void)
{
    digitalWrite(RELAYSTART,HIGH);
    g_autoOffStartMillis = millis();
    BmotorButton_Time = millis();
}

//
// motor Stop
//
void BMotorStopOperation(void)
{
    digitalWrite(RELAYSTOP,HIGH);
    BmotorButton_Time = millis();
    g_autoOffEnabled = false;
    g_autoOffStartMillis = 0u;
    remainingTime_i =0u;
}
//
// motor Start/Stop cyclic Check
//
void BMotorStartStopCyclic(void)
{
    if((millis() - BmotorButton_Time) > 1500u)
    {
        if(g_motorStatus == BMSTART)
        {
            digitalWrite(RELAYSTOP,LOW);
            g_motorStatus = BMSTOP;
        }
        else if(g_motorStatus == BMSTOP)
        {
            digitalWrite(RELAYSTART,LOW);
            g_motorStatus = BMSTART;
        }
        BmotorButton_Time = 0;
    }
}

//--------------------------------------------------
// MOTOR CONTROL
//--------------------------------------------------

void handleMotorStart(){

    if(g_motorStatus == BMSTOP)
    {
        BMotorStartOperation();
    }

    server.send(200,"text/plain","OK");
}

void handleMotorStop(){

    if(g_motorStatus == BMSTART)
    {
        BMotorStopOperation();
    }

    server.send(200,"text/plain","OK");
}

//--------------------------------------------------
// VALVE CONTROL
//--------------------------------------------------

void handleValveOpen(){

    if(g_valveStatus_u8 == MVSTOP)
    {
        sendValveCommand(true);
    }

    server.send(200,"text/plain","OK");
}

void handleValveClose(){


    if(g_valveStatus_u8 == MVSTART)
    {
        sendValveCommand(false);
    }

    server.send(200,"text/plain","OK");
}

//--------------------------------------------------
// AUTO OFF ENABLE
//--------------------------------------------------

void handleAutoOffEnable(){

    if((g_autoOffEnabled == false)&&(g_motorStatus == BMSTOP))
    {
        if(server.hasArg("mins"))
        {
            g_autoOffMinutes = server.arg("mins").toInt();
            g_autoOffEnabled = true;
        }
    }

    server.send(200,"text/plain","OK");
}

//--------------------------------------------------
// AUTO OFF DISABLE
//--------------------------------------------------

void handleAutoOffDisable(){

    if(g_motorStatus == BMSTOP)
    {
        g_autoOffEnabled = false;
    }

    server.send(200,"text/plain","OK");
}

//--------------------------------------------------
// TANK DATA -> Tank Node
//--------------------------------------------------

void handleUpdateTankData(){


    if (server.hasArg("boreEmpty"))
    {
        g_boreEmpty =
            server.arg("boreEmpty").toFloat();

        g_boreTank = tankCalculate(BORE_TANK_HEIGHT_CM,g_boreEmpty);
        
        g_boreEmpty = g_boreEmpty/30.48;
    }

    if (server.hasArg("metturEmpty"))
    {
        g_metturEmpty =
            server.arg("metturEmpty").toFloat();
        
        g_metturTank = tankCalculate(METTUR_TANK_HEIGHT_CM,g_metturEmpty);

        g_metturEmpty = g_metturEmpty/30.48;
    }

    if (server.hasArg("aliveCounter"))
    {
        g_s2Alive =
            server.arg("aliveCounter").toInt();
    }

    if (server.hasArg("ValueStatus"))
    {
        g_valveStatus_u8 =
            server.arg("ValueStatus").toInt();
    }

    server.send(200,
                "text/plain",
                "OK");
}

//--------------------------------------------------
// SETUP
//--------------------------------------------------

void setup()
{

    pinMode(RELAYSTART,OUTPUT);
    digitalWrite(RELAYSTART,LOW);
    pinMode(RELAYSTOP,OUTPUT);
    digitalWrite(RELAYSTOP,LOW);

    #ifdef SERIAL_BEGIN
    Serial.begin(9600);
    #endif

    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid,password);

    WiFi.setAutoReconnect(true);

    WiFi.persistent(true);

    WiFi.setSleep(false);

    #ifdef SERIAL_BEGIN
    Serial.println();

    Serial.print("Connecting");
    #endif

    while(WiFi.status()!=WL_CONNECTED){

        delay(500);
        #ifdef SERIAL_BEGIN
        Serial.print(".");
        #endif
    }

    #ifdef SERIAL_BEGIN
    Serial.println();

    Serial.println("Connected");

    Serial.print("IP Address : ");

    Serial.println(WiFi.localIP());
    #endif

    //--------------------------------------------------
    // ROUTES
    //--------------------------------------------------

    server.on("/",handleRoot);

    server.on("/getData",
              handleGetData);

    server.on("/bore/start",
              handleMotorStart);

    server.on("/bore/stop",
              handleMotorStop);

    server.on("/valve/open",
              handleValveOpen);

    server.on("/valve/close",
              handleValveClose);

    server.on("/autooff/enable",
              handleAutoOffEnable);

    server.on("/autooff/disable",
              handleAutoOffDisable);
    
    server.on("/updateTankData",
              handleUpdateTankData);
              

    server.begin();
    #ifdef SERIAL_BEGIN
    Serial.println("HTTP Server Started");
    #endif
}

//--------------------------------------------------
// LOOP
//--------------------------------------------------


static void VoltAmps_Updater(void)
{
    float voltage = pzem.voltage();
    float current = pzem.current();

    if (!isnan(voltage))
    {
        g_voltage = voltage;
    }

    if(!isnan(current))
    {
        g_current = current;
    }

}

unsigned long VoltAmps_UpdaterTimer = 0;

void loop(){

    server.handleClient();

    if((VoltAmps_UpdaterTimer==0)||((millis()-VoltAmps_UpdaterTimer)>800))
    {
        // AC voltage & current 
        VoltAmps_Updater();
        VoltAmps_UpdaterTimer = millis();
    }

    //--------------------------------------------------
    // AUTO OFF TIMER LOGIC
    //--------------------------------------------------

    if((g_autoOffEnabled == true)&&(g_autoOffStartMillis !=0u))
    {

        unsigned long elapsedMillis = millis() - g_autoOffStartMillis;

        unsigned long autoOffMillis = g_autoOffMinutes * 60000UL;

        if(elapsedMillis >= autoOffMillis)
        {
            BMotorStopOperation();

            #ifdef SERIAL_BEGIN
            Serial.println("AUTO OFF TRIGGERED");
            #endif
        }
        else
        {
            remainingTime_i = autoOffMillis - elapsedMillis;
        }
    }

    if(BmotorButton_Time!=0u)
    {
        BMotorStartStopCyclic();
    }


    //Serial.println(ESP.getFreeHeap());

    yield();
}

