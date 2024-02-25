#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <string>

struct configParameters {
    // Input options
    std::string rawTPX3Folder;  // Directory where raw TPX3 files are located
    std::string rawTPX3File;    // Specific TPX3 file to process
    
    // Output options
    bool writeRawSignals = false;               // Flag to write out rawsignals in binrary
    std::string outputFolder = ".";             // Default to current directory
    std::string rawSignalFile = "output";
    
    // Sorting options
    bool sortSignals = false;   // Flag to sort signals
    
    // Basic Clustering Options
    bool clusterPixels = false;
    bool writeClusters = false;
    int epsSpatial = 0;
    double epsTemporal = 0;
    int minPts = 0;

    //Diagnostic options
    int maxBuffersToRead = 0;       // Number of buffer to read in. 0 means read all buffers. 
    bool fillHistgrams = false;     // Flag to fill histograms
    int verboseLevel = 1;           // Verbosity Level
                                    // 1 = General file input/output
                                    // 2 = Config and event diagnostics
                                    // 3 = Buffer diagnostics
};

bool readConfigFile(const std::string &filename, configParameters &params);
void printParameters(const configParameters &params) ;

#endif // CONFIG_READER_H
