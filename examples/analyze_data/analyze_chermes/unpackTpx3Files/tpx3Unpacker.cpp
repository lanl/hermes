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

    auto startTime = std::chrono::high_resolution_clock::now();

    configParameters params;

    // Check for the number of command-line arguments
    if (argc < 2) {
        std::cerr << "Please provide the path to the config file as a command-line argument." << std::endl;
        return 1; // Exit the program with an error code
    }
    // Use the command-line argument for the config file path
    const char* configFilePath = argv[1];

    // Check if the configuration file was successfully read
    if(!readConfigFile(configFilePath, params)) {
        std::cerr << "Error: Failed to read configuration file: " << configFilePath << std::endl;
        return 1; // Exit the program with an error code
    } 
    // Print out config parameters based on vebosity level
    if(params.verboseLevel>=2){printParameters(params);}
    
   // If writeRawSignals is true, attempt to open an output file for writing raw signals
    ofstream rawSignalsFile;
    if (params.writeRawSignals) {
        std::string fullOutputPath = params.outputFolder + "/" + params.rawSignalFile;
        rawSignalsFile.open(fullOutputPath, ios::out | ios::binary);
        if (!rawSignalsFile) {
            cerr << "Unable to open output file!" << endl;
            return 1; // Exit with an error code if the output file cannot be opened
        }
    }

    // Construct the full path to the TPX3 file and read in the file.
    std::string fullTpx3Path = params.rawTPX3Folder + "/" + params.rawTPX3File;
    std::ifstream tpxFile(fullTpx3Path, std::ios::binary);

    if (!tpxFile) {
        cout << "This file is not found!" << endl;
    }  else {

        char* HeaderBuffer = new char[8];

        if(params.verboseLevel>=1){std::cout << "Opening TPX3 file: "<< params.rawTPX3File << std::endl;}
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

                if (params.sortSignals){
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

                // Starting grouping signals into photon events using Spatial-Temporal DBSCAN.
                if (params.clusterPixels){
                    if (params.verboseLevel>=3) {
                        std::cout <<"Buffer "<< numberOfBuffers<< ": Clustering pixels based on DBSCAN " << std::endl;
                    }
                    ST_DBSCAN(signalDataArray, signalGroupID, params.epsSpatial, params.epsTemporal, params.minPts, dataPacketsInBuffer);
                }

                //printGroupIDs(signalDataArray,signalGroupID,dataPacketsInBuffer);
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

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> totalTime = endTime - startTime;

    // Close the tpx3File
    if(params.verboseLevel>=1){std::cout << "Closing TPX3 file: "<< params.rawTPX3File << std::endl;}
    tpxFile.close();

    // If writeRawSignals is true and an output file was created, then close it. 
    if(params.writeRawSignals){
        if(params.verboseLevel>=1){std::cout << "Closing raw output file" << std::endl;}
        rawSignalsFile.close();
    }

    if(params.verboseLevel>=2){
        std::cout << std::endl << "=============== Diagnostics ==============" << std::endl;
        std::cout << "Elapsed time: " << totalTime.count() << " seconds." << std::endl;
        std::cout << "Number of headers packets: " << numberOfHeaders << std::endl;
        std::cout << "Number of TDC packets: " << numberOfTDCPackets << std::endl;
        std::cout << "Number of Pixels packets: " << numberOfPixelPackets << std::endl;
        std::cout << "Number of Global Time stamp packets: " << numberOfGlobalTSPackets << std::endl;
        std::cout << "==========================================" << std::endl;
    }

	return 0;
}



