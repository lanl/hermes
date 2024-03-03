// dataPacketProcessor.h
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <stdint.h>
#include <chrono>
#include <filesystem> // For std::filesystem::file_size

#ifndef DATAPACKETPROCESSOR_H
#define DATAPACKETPROCESSOR_H

#include "structures.h"
#include "photonRecon.h"


// Processes a TDC packet and updates the provided signalData structure.
void processTDCPacket(unsigned long long datapacket, signalData &signalData);

// Processes a Pixel packet and updates the provided signalData structure.
void processPixelPacket(unsigned long long datapacket, signalData &signalData);

// Processes a Global Time packet and updates the provided signalData structure.
void processGlobalTimePacket(unsigned long long datapacket, signalData &signalData);

// Unpack and process entire TPX3File
tpx3FileDianostics unpackAndSortEntireTPX3File(configParameters configParams);

// Unpack and process TPX3Files buffer by buffer
tpx3FileDianostics unpackandSortTPX3FileInSequentialBuffers(configParameters configParams);


#endif

