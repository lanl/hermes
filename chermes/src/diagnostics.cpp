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

void printOutUnpackingDiagnostics(configParameters configParams,tpx3FileDianostics tpxFileInfo){
    std::cout << std::endl << "=============== Diagnostics ==============" << std::endl;
    std::cout << "Total HERMES Time: " << tpxFileInfo.totalHermesTime << " seconds" << std::endl;
    std::cout << "Total Unpacking Time: " << tpxFileInfo.totalUnpackingTime << " seconds" << std::endl;
    std::cout << "Total Sorting Time: " << tpxFileInfo.totalSortingTime << " seconds" << std::endl;
    std::cout << "Total Writing Time: " << tpxFileInfo.totalWritingTime << " seconds" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "Number of headers packets: " << tpxFileInfo.numberOfBuffers << std::endl;
    std::cout << "Number of TDC packets: " << tpxFileInfo.numberOfTDC1s << std::endl;
    std::cout << "Number of Pixels packets: " << tpxFileInfo.numberOfPixelHits << std::endl;
    std::cout << "Number of Global Time stamp packets: " << tpxFileInfo.numberOfGTS << std::endl;
    std::cout << "==========================================" << std::endl;
}