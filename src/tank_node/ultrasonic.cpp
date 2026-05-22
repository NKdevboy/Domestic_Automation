#include "ultrasonic.h"

float ultrasonicReadCm(uint8_t trigPin,
                       uint8_t echoPin)
{
    uint32_t duration;

    float distanceCm;

    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(trigPin, LOW);

    duration =
        pulseIn(echoPin,
                HIGH,
                30000UL);

    distanceCm =
        duration * 0.0343f / 2.0f;

    return distanceCm;
}