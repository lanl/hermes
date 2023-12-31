#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <cstdint>

struct clusterInfo {
    int multiplicity;
    double timeDuration;
};

// Represents the data for a single signal.
struct signalData {
    int8_t signalType;    // Type of the signal (TDC=1,Pixel=2,GTS=3)
    uint8_t xPixel;       // X-coordinate of the pixel
    uint8_t yPixel;       // Y-coordinate of the pixel
    double ToaFinal;      // Time of Arrival (final value)
    double TotFinal;      // Time over Threshold (final value)
};

#endif
