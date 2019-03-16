
#ifndef UTILS_H
#define UTILS_H

#include "Definitions.h"

#define bitSet(value, bit) ((value)   |=  (1UL << (bit)))
#define getBit(value, bit) ((value)   &   (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))

float toFloat(byte intPart, byte floatPart)
{
    float f = floatPart;

    while(f >= 1.f)
    {
        f /= 10.f;
    }

    return static_cast<float>(intPart) + f;
}

#endif /* UTILS_H */
