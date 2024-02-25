#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <cstdint>

struct clusterInfo {
    int multiplicity;
    double timeDuration;
};

// Represents the data for a single raw signal.
struct signalData {
    uint8_t signalType; // Type of the signal (TDC=1,Pixel=2,GTS=3)
    uint8_t xPixel;     // X-coordinate of the pixel
    uint8_t yPixel;     // Y-coordinate of the pixel
    double ToaFinal;    // Time of Arrival in seconds (final value)
    float TotFinal;     // Time over Threshold in nano seconds (final value)
};

// Represents the data for a single reconstructed photon.
struct photonData{
    float photon_x = 0;
    float photon_y = 0;
    double photon_toa = 0;
    uint16_t integrated_tot = 0;
};

#endif
