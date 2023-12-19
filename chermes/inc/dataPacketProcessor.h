// dataPacketProcessor.h

#ifndef DATAPACKETPROCESSOR_H
#define DATAPACKETPROCESSOR_H

#include "structures.h"

// Processes a TDC packet and updates the provided signalData structure.
void processTDCPacket(unsigned long long datapacket, signalData &signalData);

// Processes a Pixel packet and updates the provided signalData structure.
void processPixelPacket(unsigned long long datapacket, signalData &signalData);

// Processes a Global Time packet and updates the provided signalData structure.
void processGlobalTimePacket(unsigned long long datapacket, signalData &signalData);

#endif

