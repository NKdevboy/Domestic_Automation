#ifndef TANK_LOGIC_H
#define TANK_LOGIC_H

#include <Arduino.h>

typedef struct
{
    float emptyHeightCm;

    float waterHeightCm;

    uint8_t percentage;

} TankData_t;

void tankCalculate(float tankHeightCm,
                   float sensorDistanceCm,
                   TankData_t* data);

#endif