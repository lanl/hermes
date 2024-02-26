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

std::string setOutputFileName(const std::string& str){
    size_t lastDotIndex = str.find_last_of('.');
        return (lastDotIndex != std::string::npos) ? str.substr(0, lastDotIndex) + ".rawSignals" : str + ".rawSignals";
}

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
            params.rawSignalFile = setOutputFileName(value);
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
            params.epsSpatial = std::stoi(value);
        } else if (key == "epsTemporal") {
            params.epsTemporal = std::stod(value);
        } else if (key == "minPts") {
            params.minPts = std::stoi(value);
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
    std::cout << "epsSpatial: " << params.epsSpatial << std::endl;
    std::cout << "epsTemporal: " << params.epsTemporal << std::endl;
    std::cout << "minPts: " << params.minPts << std::endl;
    std::cout << "=========================================================" << std::endl << std::endl;
}
