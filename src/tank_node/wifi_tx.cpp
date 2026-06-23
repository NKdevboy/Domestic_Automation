#include "wifi_tx.h"


extern float g_boreEmptyHeight;

extern float g_metturEmptyHeight;

extern uint8 g_valveStatus_u8;

/* =====================================================
   Alive Counter
===================================================== */

uint8_t g_aliveCounter = 0;


WiFiClient client;

void sendTankData(void)
{
    HTTPClient http;

    String url;

    url =
        "http://192.168.2.50/updateTankData?";

    url += "boreEmpty=" +
           String(g_boreEmptyHeight);

    url += "&metturEmpty=" +
           String(g_metturEmptyHeight);

    url += "&aliveCounter=" +
           String(g_aliveCounter);

    url += "&ValueStatus=" +
           String(g_valveStatus_u8);

    http.begin(client, url);
    http.GET();
    http.end();
       
    /* =================================================
       Alive Counter Ring Logic
    ================================================= */

       g_aliveCounter =(g_aliveCounter + 1) % 15;
}