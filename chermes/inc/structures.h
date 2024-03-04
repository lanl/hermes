#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <cstdint>
#include <string>

struct configParameters {
    // Input options
    std::string rawTPX3Folder;  // Directory where raw TPX3 files are located
    std::string rawTPX3File;    // Specific TPX3 file to process
    std::string runHandle;      // Name of the run number that is being processed.
    
    // Output options
    bool writeRawSignals = false;       // Flag to write out rawsignals in binrary
    bool writeOutPhotons = false;       // Flag to write out Photon data in binrary
    std::string outputFolder = ".";     // Default to current directory
    
    // Sorting options
    bool sortSignals = false;   // Flag to sort signals
    
    // Basic Clustering Options
    bool clusterPixels = false;
    int epsSpatial = 0;
    double epsTemporal = 0;
    int minPts = 0;

    //Diagnostic options
    uint32_t maxBuffersToRead = 0;       // Number of buffer to read in. 0 means read all buffers. 
    bool fillHistgrams = false;     // Flag to fill histograms
    int verboseLevel = 1;           // Verbosity Level
                                    // 1 = General file input/output
                                    // 2 = Config and event diagnostics
                                    // 3 = Buffer diagnostics
};

// This structure is used to contain various dianostic info used during the unpacking processes (in dataPacketProcessor class)
struct tpx3FileDiagnostics {
    uintmax_t filesize = 0;             // size in bytes of tpx3 file
    uint64_t numberOfDataPackets = 0;    // number of data packets
    uint32_t numberOfBuffers = 0;        // number of buffers 
    uint32_t numberOfPixelHits = 0;      // number of pixel hits
    uint32_t numberOfTDC1s = 0;          // number of TDC1 triggers
    uint32_t numberOfTDC2s = 0;          // number of TDC2 triggers
    uint32_t numberOfGTS = 0;            // number of global time stamps.
    uint32_t numberOfTXP3Controls = 0;   // number of TPX3 Control packets.

    double totalHermesTime = 0;
    double totalUnpackingTime = 0;
    double totalSortingTime = 0;
    double totalClusteringTime = 0;
    double totalWritingTime = 0;
};

// Represents the data for a single raw signal.
struct signalData {
    uint32_t bufferNumber;  // Buffer number from where signal was recorded
    uint8_t signalType;     // Type of the signal (TDC=1,Pixel=2,GTS=3)
    uint8_t xPixel;         // X-coordinate of the pixel
    uint8_t yPixel;         // Y-coordinate of the pixel
    double ToaFinal;        // Time of Arrival in seconds (final value)
    uint16_t TotFinal;      // Time over Threshold in nano seconds (final value)
    uint16_t groupID;       // 
};

// Represents the data for a single reconstructed photon.
struct photonData{
    float photon_x = 0;
    float photon_y = 0;
    double photon_toa = 0;
    uint16_t integrated_tot = 0;
    uint8_t multiplicity = 0;
    double timeDuration = 0;
};


/*
struct clusterInfo {
    int multiplicity;
    double timeDuration;
};*/

#endif
