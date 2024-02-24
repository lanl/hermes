// diagnostics.cpp

// User defined libraries
#include "diagnostics.h"

int numberOfHeaders = 0;
int numberOfBuffers = 0;
int numberOfTDCPackets = 0;
int numberOfPixelPackets = 0;
int numberOfGlobalTSPackets = 0;
int numberOfPhotons = 0;

void printGroupIDs(signalData* signalDataArray, int16_t* signalGroupID, size_t dataPacketsInBuffer) {
    for (size_t i = 0; i < dataPacketsInBuffer; i++) {
        std::cout << "Type: " << static_cast<int>(signalDataArray[i].signalType) 
                  << "\tPixels: " << static_cast<int>(signalDataArray[i].xPixel) 
                  << "," << static_cast<int>(signalDataArray[i].yPixel) 
                  << "\tTOA: " << signalDataArray[i].ToaFinal 
                  << "\tGroupID: " << static_cast<int>(signalGroupID[i]) 
                  << std::endl;
    }
}