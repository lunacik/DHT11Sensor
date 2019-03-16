
#ifndef UTILS_H
#define UTILS_H

#include "Definitions.h"

#ifndef bitRead
#define bitRead(value, bit) ((value)   &   (1UL << (bit)))
#endif

#ifndef bitSet
#define bitSet(value, bit) ((value)   |=  (1UL << (bit)))
#endif

#ifndef bitClear
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#endif

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