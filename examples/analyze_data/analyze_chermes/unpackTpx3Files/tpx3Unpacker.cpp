#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <algorithm>
#include <stdint.h>
#include <chrono>

// User defined functions 
#include "dataPacketProcessor.h"
#include "structures.h"
#include "diagnostics.h"
#include "photonRecon.h"
#include "configReader.h"

using namespace std;

int main(int argc, char *argv[]){  

    std::chrono::duration<double> hermesTime;
    auto hermesStartTime = std::chrono::high_resolution_clock::now();

    configParameters configParams;
    tpx3FileDianostics tpxFileInfo;


    // Check for the number of command-line arguments
    if (argc < 2) {
        std::cerr << "Please provide the path to the config file as a command-line argument." << std::endl;
        return 1; // Exit the program with an error code
    }
    // Use the command-line argument for the config file path
    const char* configFilePath = argv[1];

    // Check if the configuration file was successfully read
    if(!readConfigFile(configFilePath, configParams)) {
        std::cerr << "Error: Failed to read configuration file: " << configFilePath << std::endl;
        return 1; // Exit the program with an error code
    } 

    // Print out config parameters based on vebosity level
    if(configParams.verboseLevel>=2){printParameters(configParams);}
    
    tpxFileInfo = unpackandSortTPX3FileInSequentialBuffers(configParams);

    auto hermesStopTime = std::chrono::high_resolution_clock::now();
    hermesTime = hermesStopTime - hermesStartTime;
    tpxFileInfo.totalHermesTime = hermesTime.count();


    if(configParams.verboseLevel>=2){printOutUnpackingDiagnostics(tpxFileInfo);}

	return 0;
}



/*
// Construct the full path to the TPX3 file and read in the file.
std::string fullTpx3Path = configParams.rawTPX3Folder + "/" + configParams.rawTPX3File;
std::ifstream tpxFile(fullTpx3Path, std::ios::binary);

if (!tpxFile) {
    cout << "This file is not found!" << endl;
} else {

    char* HeaderBuffer = new char[8];

    if(configParams.verboseLevel>=1){std::cout << "Opening TPX3 file: "<< configParams.rawTPX3File << std::endl;}
    while(tpxFile.read(HeaderBuffer, 8)) {  // Read header buffer

        if (HeaderBuffer[0] == 'T' && HeaderBuffer[1] == 'P' && HeaderBuffer[2] == 'X') {
            int bufferSize = ((0xff & HeaderBuffer[7]) << 8) | (0xff & HeaderBuffer[6]);

            ++numberOfHeaders;
            //chipnr = HeaderBuffer[4];
            //mode = HeaderBuffer[5];

            // calculate the number of data packets in the buffer
            int dataPacketsInBuffer = bufferSize/8;

            // Initiate an array of datapackets that is the same size as bufferSize//8
            unsigned long long* datapackets = new unsigned long long[dataPacketsInBuffer];

            // Initiate an array of signalData called signalDataArray that is the same size as bufferSize//8
            signalData* signalDataArray = new signalData[dataPacketsInBuffer];

            // Initiate an array of group ID that is the same size 
            int16_t* signalGroupID = new int16_t[dataPacketsInBuffer];
            memset(signalGroupID, 0, dataPacketsInBuffer * sizeof(int16_t));
            
            // Read the data buffer
            tpxFile.read((char*)datapackets, bufferSize);  

            // Process each data packet in buffer and update signalDataArray
            for (int j = 0; j < dataPacketsInBuffer; j++) {
                
                //int hdr = (int)(datapackets[j] >> 56);
                int packetHeader = datapackets[j] >> 60;

                switch (packetHeader){
                case 0x6:
                    processTDCPacket(datapackets[j], signalDataArray[j]);
                    ++numberOfTDCPackets;
                    break;
                case 0xb:
                    processPixelPacket(datapackets[j], signalDataArray[j]);
                    ++numberOfPixelPackets;
                    break;
                case 0x4:
                    processGlobalTimePacket(datapackets[j], signalDataArray[j]);
                    ++numberOfGlobalTSPackets;
                    break;
                default:
                    break;

                }
            
            }

            if (configParams.sortSignals){
                // Sort the signalDataArray based on ToaFinal
                if (params.verboseLevel>=3) {
                    std::cout <<"Buffer "<< numberOfBuffers<< ": Sorting raw signal data. " << std::endl;
                }
                std::sort(signalDataArray, signalDataArray + dataPacketsInBuffer,[](const signalData &a, const signalData &b) -> bool {return a.ToaFinal < b.ToaFinal;});
            }

            if (params.writeRawSignals){
                if (params.verboseLevel>=3) {
                    std::cout <<"Buffer "<< numberOfBuffers<< ": Writing out raw signal data. " << std::endl;
                }
                if(params.writeRawSignals){
                    rawSignalsFile.write(reinterpret_cast<char*>(signalDataArray), sizeof(signalData) * dataPacketsInBuffer);
                }
            }

            // If clustering is turned on then start grouping signals into photon events using Spatial-Temporal DBSCAN.
            if (params.clusterPixels){

                // Print message for each buffer if verboselevel = 3.
                if (params.verboseLevel>=3) {
                    std::cout <<"Buffer "<< numberOfBuffers<< ": Clustering pixels based on DBSCAN " << std::endl;
                }

                // sort pixels into clusters (photons) using sorting algorith. 
                ST_DBSCAN(params, signalDataArray, signalGroupID, dataPacketsInBuffer);
            }
            
            //
            if(params.maxBuffersToRead < 250 && params.maxBuffersToRead != 0){
                //printGroupIDs(numberOfBuffers,signalDataArray,signalGroupID,dataPacketsInBuffer);
            }

            delete[] datapackets;
            delete[] signalDataArray;
            delete[] signalGroupID;
        }
        
        // Increment and check buffer count against the limit if it's non-zero
        if (params.maxBuffersToRead > 0 && ++numberOfBuffers >= params.maxBuffersToRead) {
            if (params.verboseLevel>=2) {
                std::cout << "Limit of " << params.maxBuffersToRead << " buffers reached, stopping processing." << std::endl;
            }
            break; // Exit loop if limit reached
        }
    }

    delete[] HeaderBuffer;
}
*/


