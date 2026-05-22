#include "tank_logic.h"

void tankCalculate(float tankHeightCm,
                   float sensorDistanceCm,
                   TankData_t* data)
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

    data->emptyHeightCm =
        sensorDistanceCm;

    data->waterHeightCm =
        waterHeight;

    data->percentage =
        (uint8_t)percentage;
}