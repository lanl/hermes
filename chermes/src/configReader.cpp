#include "configReader.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool readConfigFile(const std::string &filename, configParameters &params) {
    std::ifstream configFile(filename);

    if (!configFile.is_open()) {
        std::cerr << "Failed to open configuration file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        // Ignore lines that start with '#' or contain '#'
        if (line.empty() || line[0] == '#' || line.find('#') != std::string::npos) {
            continue;
        }

        std::istringstream lineStream(line);
        std::string key;
        std::string value;

        std::getline(lineStream, key, '=');
        std::getline(lineStream, value);

        if (key == "inputTPX3File") {
            params.tpxFileName = value;
        } else if (key == "writeRawSignals") {
            if (value == "true") {
                params.writeRawSignals = true;
            } else if (value == "false") {
                params.writeRawSignals = false;
            } else {
                std::cerr << ">CONFIG ERROR: Invalid value of '"<< value <<"' for writeRawSignals in config file." << std::endl;
                std::cerr << ">CONFIG ERROR: Expected 'true' or 'false'. Setting to defualt value" << std::endl;
            }
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
        } else if (key == "verbose") {
            if (value == "true") {
                params.verbose = true;
            } else if (value == "false") {
                params.verbose = false;
            } else {
                std::cerr << ">CONFIG ERROR: Invalid value of '"<< value <<"' for verbose in config file." << std::endl;
                std::cerr << ">CONFIG ERROR: Expected 'true' or 'false'. Setting to defualt value" << std::endl;
            }
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

void printParameters(configParameters &params){
    //If the program reaches this point, the configuration file was successfully read
    std::cout << "Config parameters =======================================" << std::endl;
    std::cout << "inputTPX3File: " << params.tpxFileName << std::endl;
    std::cout << "epsSpatial: " << params.epsSpatial << std::endl;
    std::cout << "epsTemporal: " << params.epsTemporal << std::endl;
    std::cout << "minPts: " << params.minPts << std::endl;
    std::cout << "=========================================================" << std::endl;
}
