#include "configReader.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Trim function to remove leading and trailing whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
        return ""; // Return an empty string if the string contains only spaces
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

std::string grabRunHandle(const std::string& str){
    size_t lastDotIndex = str.find_last_of('.');
        return (lastDotIndex != std::string::npos) ? str.substr(0, lastDotIndex) : str;
}


/**
 * @brief Reads configuration parameters from a given file and populates a struct with the configuration values.
 * 
 * This function opens a configuration file specified by the filename and reads it line by line. Each line
 * is expected to contain a key-value pair separated by an '=' character. Lines starting with '#' or containing
 * '#' are ignored as comments. The function supports setting various configuration parameters specified
 * by the keys in the file. Invalid or unexpected key-value pairs will generate errors to stderr.
 * 
 * Supported configuration parameters include:
 * - rawTPX3Folder: Folder for raw TPX3 files.
 * - rawTPX3File: Specific raw TPX3 file name.
 * - writeRawSignals: Whether to write raw signals (true/false).
 * - outputFolder: Folder for output files.
 * - maxBuffersToRead: Maximum number of buffers to read.
 * - sortSignals: Whether to sort signals (true/false).
 * - verboseLevel: Level of verbosity in output.
 * - fillHistgrams: Whether to fill histograms (true/false).
 * - clusterPixels: Whether to cluster pixels (true/false).
 * - queryRegion: Region to query (as an integer within uint16_t range).
 * - writeOutPhotons: Whether to write out photons (true/false).
 * - epsSpatial: Epsilon spatial value (as an integer within uint8_t range).
 * - epsTemporal: Epsilon temporal value (as a double).
 * - minPts: Minimum points value (as an integer within uint8_t range).
 * 
 * TODO: refactor this!!!
 * 
 * @param filename The path to the configuration file to be read.
 * @param params Reference to a struct where the configuration parameters will be stored.
 * @return true if the file was successfully read and parsed; false otherwise.
 * 
 * @note The function will print error messages to stderr for any issues encountered while reading the file
 * or parsing the configuration parameters.
 */
bool readConfigFile(const std::string &filename, configParameters &params) {
    std::ifstream configFile(filename); // Attempt to open the configuration file
    std::string line;                   // Container for line
    std::string key;                    // Container for key string
    std::string value;                  // Container for value string

    // Check if the file was successfully opened
    if (!configFile.is_open()) {
        std::cerr << "Failed to open configuration file: " << filename << std::endl;
        return false;
    }
    
    // Read the file line by line
    while (std::getline(configFile, line)) {
        // Ignore lines that start with '#' or contain '#'
        if (line.empty() || line[0] == '#' || line.find('#') != std::string::npos) {
            continue;
        }
        
        // Use a string stream to separate the key and value
        std::istringstream lineStream(line);   
        // Split the line into key and value at the '=' character
        std::getline(lineStream, key, '=');
        std::getline(lineStream, value);

        // Trim the key and value here
        key = trim(key);
        value = trim(value);

        // Based on key, set value of parameters
        if (key == "rawTPX3Folder") {
            params.rawTPX3Folder = value;
        } else if (key == "rawTPX3File") {
            params.rawTPX3File = value;
            params.runHandle = grabRunHandle(value);
        } else if (key == "writeRawSignals") {
            if (value == "true") {
                params.writeRawSignals = true;
            } else if (value == "false") {
                params.writeRawSignals = false;
            } else {
                std::cerr << ">CONFIG ERROR: Invalid value of '"<< value <<"' for writeRawSignals in config file." << std::endl;
                std::cerr << ">CONFIG ERROR: Expected 'true' or 'false'. Setting to defualt value" << std::endl;
            }
        } else if (key == "outputFolder") {
            params.outputFolder = value;
        } else if (key == "maxBuffersToRead") {
            params.maxBuffersToRead = std::stoi(value);
        } else if (key == "sortSignals") {
            if (value == "true") {
                params.sortSignals = true;
            } else if (value == "false") {
                params.sortSignals = false;
            } else {
                std::cerr << ">CONFIG ERROR: Invalid value of '"<< value <<"' for sortSignals in config file." << std::endl;
                std::cerr << ">CONFIG ERROR: Expected 'true' or 'false'. Setting to defualt value" << std::endl;
            }
        } else if (key == "verboseLevel") {
            params.verboseLevel = std::stoi(value);
        } else if (key == "fillHistgrams") {
            if (value == "true") {
                params.fillHistgrams = true;
            } else if (value == "false") {
                params.fillHistgrams = false;
            } else {
                std::cerr << ">CONFIG ERROR: Invalid value of '"<< value <<"' for fill Histgrams in config file." << std::endl;
                std::cerr << ">CONFIG ERROR: Expected 'true' or 'false'. Setting to defualt value" << std::endl;
            }
        } else if (key == "clusterPixels") {
            if (value == "true") {
                params.clusterPixels = true;
            } else if (value == "false") {
                params.clusterPixels = false;
            } else {
                std::cerr << ">CONFIG ERROR: Invalid value of '"<< value <<"' for clusterPixels in config file." << std::endl;
                std::cerr << ">CONFIG ERROR: Expected 'true' or 'false'. Setting to defualt value" << std::endl;
            }
        } else if (key == "queryRegion") {
            try {
                int temp = std::stoi(value); // Convert string to int
                if (temp >= 0 && temp <= UINT16_MAX) {
                    params.queryRegion = static_cast<uint16_t>(temp);
                } else {
                    std::cerr << "Error: queryRegion value out of uint16_t range: " << value << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Exception converting queryRegion: " << e.what() << std::endl;
            }
        } else if (key == "writeOutPhotons") {
            if (value == "true") {
                params.writeOutPhotons = true;
            } else if (value == "false") {
                params.writeOutPhotons = false;
            } else {
                std::cerr << ">CONFIG ERROR: Invalid value of '"<< value <<"' for writeOutPhotons in config file." << std::endl;
                std::cerr << ">CONFIG ERROR: Expected 'true' or 'false'. Setting to defualt value" << std::endl;
            } 
        } else if (key == "epsSpatial") {
            try {
                int temp = std::stoi(value); // Convert string to int
                if (temp >= 0 && temp <= UINT8_MAX) {
                    params.epsSpatial = static_cast<uint8_t>(temp);
                } else {
                    std::cerr << "Error: epsSpatial value out of uint8_t range: " << value << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Exception converting epsSpatial: " << e.what() << std::endl;
            }
        } else if (key == "epsTemporal") {
            params.epsTemporal = std::stod(value);
        } else if (key == "minPts") {
            try {
                int temp = std::stoi(value); // Convert string to int
                if (temp >= 0 && temp <= UINT8_MAX) {
                    params.minPts = static_cast<uint8_t>(temp);
                } else {
                    std::cerr << "Error: minPts value out of uint8_t range: " << value << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Exception converting minPts: " << e.what() << std::endl;
            }
        }
    }

    configFile.close();
    return true;
}

void printParameters(const configParameters &params) {
    // If the program reaches this point, the configuration file was successfully read
    std::cout << "=================== Config parameters ====================" << std::endl;
    std::cout << "inputTPX3Folder: " << params.rawTPX3Folder << std::endl;
    std::cout << "inputTPX3File: " << params.rawTPX3File << std::endl;
    std::cout << "writeRawSignals: " << (params.writeRawSignals ? "true" : "false") << std::endl;
    std::cout << "outputFolder: " << params.outputFolder << std::endl;
    std::cout << "maxBuffersToRead: " << params.maxBuffersToRead << std::endl;
    std::cout << "sortSignals: " << (params.sortSignals ? "true" : "false") << std::endl;
    std::cout << "verboseLevel: " << params.verboseLevel << std::endl;
    std::cout << "fillHistgrams: " << (params.fillHistgrams ? "true" : "false") << std::endl;
    std::cout << "clusterPixels: " << (params.clusterPixels ? "true" : "false") << std::endl;
    std::cout << "writeOutPhotons: " << (params.writeOutPhotons ? "true" : "false") << std::endl;
    std::cout << "epsSpatial: " << static_cast<int>(params.epsSpatial) << std::endl;
    std::cout << "epsTemporal: " << params.epsTemporal << std::endl;
    std::cout << "minPts: " << static_cast<int>(params.minPts) << std::endl;
    std::cout << "=========================================================" << std::endl << std::endl;
}
