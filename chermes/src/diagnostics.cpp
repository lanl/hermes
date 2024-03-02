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

/**
 * @brief Prints out group ID info for all data in a single TPX3 data buffer.
 *
 * This function function loops through the singalData and the corresponding signalGroupID arrays
 * and prints out a table of signalType, xPixel, yPixel, ToaFinal, TotFinal, and groupID. 
 * Here groupID is assigned from some sorting algorithm, such as DBSCAN. 
 *
 * TODO: Add a output file instead of printing to terminal. 
 * 
 * @param buffernNumber         the buffer number from which the singalData was taking from.
 * @param signalDataArray       the array of raw signal data from a TPX3 file, such as pixelHit, TDCs, or global time stamps.
 * @param signalGroupID         the corresponding sorted IDs for all the raw signal data.
 * @param dataPacketsInBuffer   Number of data packets in buffer. 
 * @return nothing
 */
void printGroupIDs(int buffernNumber, signalData* signalDataArray, int16_t* signalGroupID, size_t dataPacketsInBuffer) {
    // Loop through dataPacketsInBuffer and print out each field. 
    for (size_t i = 0; i < dataPacketsInBuffer; i++) {
        std::cout << std::left 
                  << std::setw(6) << buffernNumber
                  << std::setw(10) << signalTypeToString(static_cast<int>(signalDataArray[i].signalType))
                  << std::setw(8) << static_cast<int>(signalDataArray[i].xPixel)
                  << std::setw(8) << static_cast<int>(signalDataArray[i].yPixel)
                  << std::setw(16) << std::fixed << std::setprecision(10) << signalDataArray[i].ToaFinal
                  << std::setw(10) << std::fixed << std::setprecision(3) << signalDataArray[i].TotFinal
                  << std::setw(10) << static_cast<int>(signalGroupID[i])
                  << std::endl;
    }
}

/**
 * @brief Prints out all diagnostics for unpacking, sorting, and writingout data from a TPX3 file.
 *
 * This function function takes a HERMES defined structure tpx3FileDianostics
 * and prints out most of the diagnostic info that might be desired in 
 *
 * @param tpxFileInfo a tpx3FileDianostics structure that contains all the diagnostic containers for
 * unpacking and processing tpx3Files.
 * @return nothing
 */
void printOutUnpackingDiagnostics(tpx3FileDianostics tpxFileInfo){
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