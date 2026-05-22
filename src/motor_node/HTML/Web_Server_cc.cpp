/* =========================================================
   web_server.cpp
========================================================= */

#include "web_server.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

/* =========================================================
   Web Server Object
========================================================= */

ESP8266WebServer server(80);

/* =========================================================
   Global Variables
========================================================= */

/* ---------------- Supply Status ---------------- */

float g_supplyVoltage = 230.0;

float g_supplyCurrent = 5.2;

/* ---------------- Bore Motor ---------------- */

bool g_boreMotorStatus = false;

/* ---------------- Bore Tank ---------------- */

int g_boreTankLevel = 0;

float g_boreEmptyHeightCm = 0;

/* ---------------- Mettur Valve ---------------- */

bool g_metturValveStatus = false;

/* ---------------- Mettur Tank ---------------- */

int g_metturTankLevel = 0;

float g_metturEmptyHeightCm = 0;

/* ---------------- Auto OFF ---------------- */

uint32_t g_boreAutoOffTimeMin = 20;

bool g_boreAutoOffEnabled = false;

bool g_autoOffTimerRunning = false;

uint32_t g_motorStartTimeMs = 0;

uint32_t g_boreAutoOffRemainingSec = 0;

/* =========================================================
   Tank Node Monitoring
========================================================= */

uint8_t g_currentAliveCounter = 0;

uint8_t g_previousAliveCounter = 0;

uint8_t g_aliveCounterMissCount = 0;

bool g_tankNodeOnline = false;

uint32_t g_lastAliveCheckMs = 0;

/* =========================================================
   HTML Page
========================================================= */

String createHTMLPage(void)
{
    String motorStatus =
        (g_boreMotorStatus == true)
        ? "STARTED"
        : "STOPPED";

    String valveStatus =
        (g_metturValveStatus == true)
        ? "OPEN"
        : "CLOSED";

    String autoOffStatus =
        (g_boreAutoOffEnabled == true)
        ? "ENABLED"
        : "DISABLED";

    String tankNodeStatus =
        (g_tankNodeOnline == true)
        ? "ONLINE"
        : "SNA";

    String remainingTimeString;

    if (g_autoOffTimerRunning == true)
    {
        uint32_t minutes =
            g_boreAutoOffRemainingSec / 60UL;

        uint32_t seconds =
            g_boreAutoOffRemainingSec % 60UL;

        remainingTimeString =
            String(minutes) + "m " +
            String(seconds) + "s";
    }
    else
    {
        remainingTimeString = "--";
    }

    String boreTankText;

    String metturTankText;

    String boreEmptyText;

    String metturEmptyText;

    if (g_tankNodeOnline == true)
    {
        boreTankText =
            String(g_boreTankLevel);

        metturTankText =
            String(g_metturTankLevel);

        boreEmptyText =
            String(g_boreEmptyHeightCm);

        metturEmptyText =
            String(g_metturEmptyHeightCm);
    }
    else
    {
        boreTankText = "SNA";

        metturTankText = "SNA";

        boreEmptyText = "SNA";

        metturEmptyText = "SNA";
    }

    String html = R"rawliteral(

<!DOCTYPE html>
<html>

<head>

<meta name="viewport"
      content="width=device-width, initial-scale=1">

<style>

body
{
    font-family: Arial;
    background: #f2f2f2;
    margin: 0;
    padding: 10px;
}

.card
{
    background: white;
    border-radius: 15px;
    padding: 15px;
    margin-bottom: 15px;
    box-shadow: 0px 2px 8px rgba(0,0,0,0.15);
}

h2
{
    margin-top: 0;
}

button
{
    width: 45%;
    padding: 12px;
    margin: 5px;
    border: none;
    border-radius: 10px;
    font-size: 16px;
    color: white;
}

.startBtn
{
    background: green;
}

.stopBtn
{
    background: red;
}

.status
{
    margin-top: 10px;
    font-size: 17px;
    font-weight: bold;
}

.tankLevel
{
    font-size: 32px;
    text-align: center;
    color: blue;
    font-weight: bold;
}

.timerInput
{
    width: 90px;
    padding: 10px;
}

</style>

</head>

<body>

<!-- =====================================================
     Supply Status
===================================================== -->

<div class="card">

<h2>Supply Status</h2>

<div class="status">
Voltage :
<span id="voltage">%VOLTAGE%</span> V
</div>

<div class="status">
Current :
<span id="current">%CURRENT%</span> A
</div>

</div>

<!-- =====================================================
     Bore Motor
===================================================== -->

<div class="card">

<h2>Bore Motor</h2>

<button class="startBtn"
onclick="location.href='/bore/start'">
START
</button>

<button class="stopBtn"
onclick="location.href='/bore/stop'">
STOP
</button>

<div class="status">
Motor Status :
<span id="motorStatus">%MOTOR_STATUS%</span>
</div>

<hr>

<h3>Auto OFF</h3>

<form action="/bore/autooff" method="GET">

<input type="number"
name="time"
value="%AUTO_OFF_TIME%"
class="timerInput">

Minutes

<br><br>

<button type="submit"
class="startBtn"
style="width:100%;">
SET TIMER
</button>

</form>

<div class="status">
Auto OFF :
<span id="autoOffStatus">
%AUTO_OFF_STATUS%
</span>
</div>

<div class="status">
Remaining :
<span id="remainingTime">
%REMAINING_TIME%
</span>
</div>

</div>

<!-- =====================================================
     Bore Tank
===================================================== -->

<div class="card">

<h2>Bore Tank</h2>

<div class="tankLevel">
<span id="boreTank">%BORE_TANK%</span>
</div>

<div class="status">
Empty Height :
<span id="boreEmpty">%BORE_EMPTY%</span>
</div>

<div class="status">
Tank Node :
<span id="tankNodeStatus">
%TANK_NODE_STATUS%
</span>
</div>

</div>

<!-- =====================================================
     Mettur Valve
===================================================== -->

<div class="card">

<h2>Mettur Valve</h2>

<button class="startBtn"
onclick="location.href='/valve/open'">
OPEN
</button>

<button class="stopBtn"
onclick="location.href='/valve/close'">
CLOSE
</button>

<div class="status">
Valve Status :
<span id="valveStatus">%VALVE_STATUS%</span>
</div>

</div>

<!-- =====================================================
     Mettur Tank
===================================================== -->

<div class="card">

<h2>Mettur Tank</h2>

<div class="tankLevel">
<span id="metturTank">%METTUR_TANK%</span>
</div>

<div class="status">
Empty Height :
<span id="metturEmpty">%METTUR_EMPTY%</span>
</div>

</div>

<script>

setInterval(function()
{
    fetch('/getData')
    .then(response => response.json())
    .then(data =>
    {
        document.getElementById("voltage").innerHTML =
            data.voltage;

        document.getElementById("current").innerHTML =
            data.current;

        document.getElementById("motorStatus").innerHTML =
            data.motorStatus;

        document.getElementById("boreTank").innerHTML =
            data.boreTank;

        document.getElementById("boreEmpty").innerHTML =
            data.boreEmpty;

        document.getElementById("valveStatus").innerHTML =
            data.valveStatus;

        document.getElementById("metturTank").innerHTML =
            data.metturTank;

        document.getElementById("metturEmpty").innerHTML =
            data.metturEmpty;

        document.getElementById("autoOffStatus").innerHTML =
            data.autoOffStatus;

        document.getElementById("remainingTime").innerHTML =
            data.remainingTime;

        document.getElementById("tankNodeStatus").innerHTML =
            data.tankNodeStatus;
    });

}, 2000);

</script>

</body>
</html>

)rawliteral";

    html.replace("%VOLTAGE%",
                 String(g_supplyVoltage));

    html.replace("%CURRENT%",
                 String(g_supplyCurrent));

    html.replace("%MOTOR_STATUS%",
                 motorStatus);

    html.replace("%BORE_TANK%",
                 boreTankText);

    html.replace("%BORE_EMPTY%",
                 boreEmptyText);

    html.replace("%VALVE_STATUS%",
                 valveStatus);

    html.replace("%METTUR_TANK%",
                 metturTankText);

    html.replace("%METTUR_EMPTY%",
                 metturEmptyText);

    html.replace("%AUTO_OFF_TIME%",
                 String(g_boreAutoOffTimeMin));

    html.replace("%AUTO_OFF_STATUS%",
                 autoOffStatus);

    html.replace("%REMAINING_TIME%",
                 remainingTimeString);

    html.replace("%TANK_NODE_STATUS%",
                 tankNodeStatus);

    return html;
}

/* =========================================================
   Receive Tank Data
========================================================= */

void handleUpdateTankData(void)
{
    if (server.hasArg("borePercent"))
    {
        g_boreTankLevel =
            server.arg("borePercent").toInt();
    }

    if (server.hasArg("boreEmpty"))
    {
        g_boreEmptyHeightCm =
            server.arg("boreEmpty").toFloat();
    }

    if (server.hasArg("metturPercent"))
    {
        g_metturTankLevel =
            server.arg("metturPercent").toInt();
    }

    if (server.hasArg("metturEmpty"))
    {
        g_metturEmptyHeightCm =
            server.arg("metturEmpty").toFloat();
    }

    if (server.hasArg("aliveCounter"))
    {
        g_currentAliveCounter =
            server.arg("aliveCounter").toInt();
    }

    if (g_currentAliveCounter !=
        g_previousAliveCounter)
    {
        g_previousAliveCounter =
            g_currentAliveCounter;

        g_aliveCounterMissCount = 0;

        g_tankNodeOnline = true;
    }

    server.send(200,
                "text/plain",
                "OK");
}

/* =========================================================
   JSON Data
========================================================= */

void handleGetData(void)
{
    String motorStatus =
        (g_boreMotorStatus == true)
        ? "STARTED"
        : "STOPPED";

    String valveStatus =
        (g_metturValveStatus == true)
        ? "OPEN"
        : "CLOSED";

    String autoOffStatus =
        (g_boreAutoOffEnabled == true)
        ? "ENABLED"
        : "DISABLED";

    String tankNodeStatus =
        (g_tankNodeOnline == true)
        ? "ONLINE"
        : "SNA";

    String remainingTimeString;

    if (g_autoOffTimerRunning == true)
    {
        uint32_t minutes =
            g_boreAutoOffRemainingSec / 60UL;

        uint32_t seconds =
            g_boreAutoOffRemainingSec % 60UL;

        remainingTimeString =
            String(minutes) + "m " +
            String(seconds) + "s";
    }
    else
    {
        remainingTimeString = "--";
    }

    String boreTankText;

    String metturTankText;

    String boreEmptyText;

    String metturEmptyText;

    if (g_tankNodeOnline == true)
    {
        boreTankText =
            String(g_boreTankLevel);

        metturTankText =
            String(g_metturTankLevel);

        boreEmptyText =
            String(g_boreEmptyHeightCm);

        metturEmptyText =
            String(g_metturEmptyHeightCm);
    }
    else
    {
        boreTankText = "SNA";

        metturTankText = "SNA";

        boreEmptyText = "SNA";

        metturEmptyText = "SNA";
    }

    String json = "{";

    json += "\"voltage\":\"" +
            String(g_supplyVoltage) + "\",";

    json += "\"current\":\"" +
            String(g_supplyCurrent) + "\",";

    json += "\"motorStatus\":\"" +
            motorStatus + "\",";

    json += "\"boreTank\":\"" +
            boreTankText + "\",";

    json += "\"boreEmpty\":\"" +
            boreEmptyText + "\",";

    json += "\"valveStatus\":\"" +
            valveStatus + "\",";

    json += "\"metturTank\":\"" +
            metturTankText + "\",";

    json += "\"metturEmpty\":\"" +
            metturEmptyText + "\",";

    json += "\"autoOffStatus\":\"" +
            autoOffStatus + "\",";

    json += "\"remainingTime\":\"" +
            remainingTimeString + "\",";

    json += "\"tankNodeStatus\":\"" +
            tankNodeStatus + "\"";

    json += "}";

    server.send(200,
                "application/json",
                json);
}

/* =========================================================
   Root
========================================================= */

void handleRoot(void)
{
    server.send(200,
                "text/html",
                createHTMLPage());
}

/* =========================================================
   Bore Motor
========================================================= */

void handleBoreStart(void)
{
    onBoreMotorStart();

    g_boreMotorStatus = true;

    g_motorStartTimeMs = millis();

    if (g_boreAutoOffEnabled == true)
    {
        g_autoOffTimerRunning = true;

        g_boreAutoOffRemainingSec =
            g_boreAutoOffTimeMin * 60UL;
    }

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleBoreStop(void)
{
    onBoreMotorStop();

    g_boreMotorStatus = false;

    g_autoOffTimerRunning = false;

    g_boreAutoOffRemainingSec = 0;

    server.sendHeader("Location", "/");
    server.send(303);
}

/* =========================================================
   Auto OFF
========================================================= */

void handleBoreAutoOff(void)
{
    if (server.hasArg("time"))
    {
        g_boreAutoOffTimeMin =
            server.arg("time").toInt();

        g_boreAutoOffEnabled = true;

        if (g_boreMotorStatus == true)
        {
            g_motorStartTimeMs = millis();

            g_autoOffTimerRunning = true;

            g_boreAutoOffRemainingSec =
                g_boreAutoOffTimeMin * 60UL;
        }
    }

    server.sendHeader("Location", "/");
    server.send(303);
}

/* =========================================================
   Valve
========================================================= */

void handleValveOpen(void)
{
    onMetturValveOpen();

    g_metturValveStatus = true;

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleValveClose(void)
{
    onMetturValveClose();

    g_metturValveStatus = false;

    server.sendHeader("Location", "/");
    server.send(303);
}

/* =========================================================
   Init
========================================================= */

void webServerInit(void)
{
    server.on("/", handleRoot);

    server.on("/getData",
              handleGetData);

    server.on("/updateTankData",
              handleUpdateTankData);

    server.on("/bore/start",
              handleBoreStart);

    server.on("/bore/stop",
              handleBoreStop);

    server.on("/bore/autooff",
              handleBoreAutoOff);

    server.on("/valve/open",
              handleValveOpen);

    server.on("/valve/close",
              handleValveClose);

    server.begin();

    Serial.println("Web Server Started");
}

/* =========================================================
   Handle Client
========================================================= */

void webServerHandleClient(void)
{
    static uint32_t previousTickMs = 0;

    server.handleClient();

    /* ==============================================
       Auto OFF Countdown
    ============================================== */

    if ((millis() - previousTickMs) >= 1000UL)
    {
        previousTickMs = millis();

        if ((g_autoOffTimerRunning == true) &&
            (g_boreAutoOffRemainingSec > 0))
        {
            g_boreAutoOffRemainingSec--;
        }
    }

    /* ==============================================
       Auto OFF Logic
    ============================================== */

    if ((g_boreMotorStatus == true) &&
        (g_boreAutoOffEnabled == true))
    {
        if ((millis() - g_motorStartTimeMs) >=
            (g_boreAutoOffTimeMin * 60000UL))
        {
            onBoreMotorStop();

            g_boreMotorStatus = false;

            g_autoOffTimerRunning = false;

            g_boreAutoOffRemainingSec = 0;

            g_boreAutoOffEnabled = false;
        }
    }

    /* ==============================================
       Tank Node Timeout Monitoring
    ============================================== */

    if ((millis() - g_lastAliveCheckMs) >= 5000UL)
    {
        g_lastAliveCheckMs = millis();

        g_aliveCounterMissCount++;

        if (g_aliveCounterMissCount >= 3)
        {
            g_tankNodeOnline = false;
        }
    }
}

/* =========================================================
   Callback Functions
========================================================= */

void onBoreMotorStart(void)
{
    Serial.println("Bore Motor START");
}

void onBoreMotorStop(void)
{
    Serial.println("Bore Motor STOP");
}

void onMetturValveOpen(void)
{
    Serial.println("Valve OPEN");
}

void onMetturValveClose(void)
{
    Serial.println("Valve CLOSE");
}