#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

float ultrasonicReadCm(uint8_t trigPin,
                       uint8_t echoPin);

#endif