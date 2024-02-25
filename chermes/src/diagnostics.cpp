#include <iomanip> // Include for std::setw and std::setprecision

// User defined libraries
#include "diagnostics.h"

int numberOfHeaders = 0;
int numberOfBuffers = 0;
int numberOfTDCPackets = 0;
int numberOfPixelPackets = 0;
int numberOfGlobalTSPackets = 0;
int numberOfPhotons = 0;

std::string signalTypeToString(int signalType) {
    switch (signalType) {
        case 1: return "TDC";
        case 2: return "Pixel";
        case 3: return "GTS";
        default: return "Unknown";
    }
}

void printGroupIDs(int numberOfBuffers, signalData* signalDataArray, int16_t* signalGroupID, size_t dataPacketsInBuffer) {
    // Loop through dataPacketsInBuffer and print out each field. 
    for (size_t i = 0; i < dataPacketsInBuffer; i++) {
        std::cout << std::left 
                  << std::setw(6) << numberOfBuffers
                  << std::setw(10) << signalTypeToString(static_cast<int>(signalDataArray[i].signalType))
                  << std::setw(8) << static_cast<int>(signalDataArray[i].xPixel)
                  << std::setw(8) << static_cast<int>(signalDataArray[i].yPixel)
                  << std::setw(16) << std::fixed << std::setprecision(10) << signalDataArray[i].ToaFinal
                  << std::setw(10) << std::fixed << std::setprecision(3) << signalDataArray[i].TotFinal
                  << std::setw(10) << static_cast<int>(signalGroupID[i])
                  << std::endl;
    }
}