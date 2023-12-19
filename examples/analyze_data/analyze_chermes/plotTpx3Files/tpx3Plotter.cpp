#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <algorithm>
#include <stdint.h>
#include <chrono>

// ROOT headers
#include <TFile.h>


// User defined functions 
#include "dataPacketProcessor.h"
#include "structures.h"
#include "diagnostics.h"
#include "histograms.h"
#include "photonRecon.h"
#include "configReader.h"

using namespace std;

int main(int argc, char *argv[]){  

    auto startTime = std::chrono::high_resolution_clock::now();

    configParameters params;

    if (readConfigFile("config.txt", params)) {
        std::cout << "inputTPX3File: " << params.tpxFileName << std::endl;
        std::cout << "epsSpatial: " << params.epsSpatial << std::endl;
        std::cout << "epsTemporal: " << params.epsTemporal << std::endl;
        std::cout << "minPts: " << params.minPts << std::endl;
    }

    std::ifstream tpxFile(params.tpxFileName, std::ios::binary);
    ofstream rawSignalsFile("output/rawSignals.bin", ios::out | ios::binary);
    TFile *rootFile = new TFile("output/histograms.root", "RECREATE");
    
    if (!rawSignalsFile) {
    	cerr << "Unable to open output file!" << endl;
    	return 1;
	}
    if (!tpxFile) {
        cout << "This file is not found!" << endl;
    }  else {

        char* HeaderBuffer = new char[8];

        std::cout << "Opening TPX3 file =======================================" << std::endl; 
        while(tpxFile.read(HeaderBuffer, 8)) {  // Read header buffer

            if (HeaderBuffer[0] == 'T' && HeaderBuffer[1] == 'P' && HeaderBuffer[2] == 'X') {
                int bufferSize = ((0xff & HeaderBuffer[7]) << 8) | (0xff & HeaderBuffer[6]);

                cout << "============= Reading in buffer of size: " << bufferSize << "\t============"<< endl;
                
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
                        if (params.fillHistgrams){fillRawTDCHistogram(signalDataArray[j]);}
                        ++numberOfTDCPackets;
            			break;
        			case 0xb:
			            processPixelPacket(datapackets[j], signalDataArray[j]);
                        if (params.fillHistgrams){fillRawPixelHistogram(signalDataArray[j]);}
                        ++numberOfPixelPackets;
			            break;
			        case 0x4:
			            processGlobalTimePacket(datapackets[j], signalDataArray[j]);
                        if (params.fillHistgrams){fillRawGTSHistogram(signalDataArray[j]);}
                        ++numberOfGlobalTSPackets;
			            break;
			        default:
			            break;

                    }
                
                }

                if (params.sortSignals){
                    // Sort the signalDataArray based on ToaFinal
                    std::cout << "Sorting signals in buffer. " << std::endl;
                    std::sort(signalDataArray, signalDataArray + dataPacketsInBuffer,[](const signalData &a, const signalData &b) -> bool {return a.ToaFinal < b.ToaFinal;});
                }

                if (params.writeRawSignals){
                    // Write out the raw signals file.
                    std::cout << "Writing out raw signal data. " << std::endl;
                    rawSignalsFile.write(reinterpret_cast<char*>(signalDataArray), sizeof(signalData) * dataPacketsInBuffer);
                }

                // Starting grouping signals into photon events using Spatial-Temporal DBSCAN.
                ST_DBSCAN(signalDataArray, signalGroupID, params.epsSpatial, params.epsTemporal, params.minPts, dataPacketsInBuffer);


                //printGroupIDs(signalDataArray,signalGroupID,dataPacketsInBuffer);

                delete[] datapackets;
                delete[] signalDataArray;
                delete[] signalGroupID;
            }
        }

        delete[] HeaderBuffer;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> totalTime = endTime - startTime;

    std::cout << "========================================== Closing TPX3 file" << std::endl;
    tpxFile.close();
    std::cout << "========================================= Writing Histograms" << std::endl;
    writeHistograms();
    std::cout << "==================================== Closing raw output file" << std::endl;
    rawSignalsFile.close();
    std::cout << "=================================== Closing root output file" << std::endl;
    rootFile->Close();

    std::cout << std::endl << "============= Diagnostics ============" << std::endl;
    std::cout << "Elapsed time: " << totalTime.count() << " seconds." << std::endl;
    std::cout << "Number of headers packets: " << numberOfHeaders << std::endl;
    std::cout << "Number of TDC packets: " << numberOfTDCPackets << std::endl;
    std::cout << "Number of Pixels packets: " << numberOfPixelPackets << std::endl;
    std::cout << "Number of Global Time stamp packets: " << numberOfGlobalTSPackets << std::endl;
    std::cout << "==========================================" << std::endl;
 

	return 0;
}



