/* =========================================================
   web_server.h
========================================================= */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

/* =========================================================
   Supply Status
========================================================= */
extern float g_supplyVoltage;
extern float g_supplyCurrent;

/* =========================================================
   Bore Motor
========================================================= */
extern bool g_boreMotorStatus;

/* =========================================================
   Bore Tank
========================================================= */
extern int g_boreTankLevel;

/* =========================================================
   Mettur Valve
========================================================= */
extern bool g_metturValveStatus;

/* =========================================================
   Mettur Tank
========================================================= */
extern int g_metturTankLevel;

/* =========================================================
   Bore Motor Auto OFF
========================================================= */
extern uint32_t g_boreAutoOffTimeMin;
extern bool g_boreAutoOffEnabled;

/* =========================================================
   User Callback Functions
========================================================= */
void onBoreMotorStart(void);
void onBoreMotorStop(void);

void onMetturValveOpen(void);
void onMetturValveClose(void);

/* =========================================================
   Web Server
========================================================= */
void webServerInit(void);
void webServerHandleClient(void);

#endif