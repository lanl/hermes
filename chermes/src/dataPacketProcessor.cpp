// dataPacketProcessor.cpp
#include <filesystem>

#include "dataPacketProcessor.h"
#include "structures.h"

using namespace std;

const double NANOSECS = 1E-9;

// Some global variables, constants, and containers
unsigned long long fileLength;
unsigned long long NumofPackets;

// TODO: Fix this! Possibly make it into a structure that is inherited 
double coarseTime;
unsigned long tmpfine;
unsigned long trigTimeFine;
double timeUnit = 25. / 4096; 
double global_timestamp;
int trigger_counter;
unsigned long long int timemaster;
double TDC_timestamp;
double spidrTimens;
int x, y;
double timeOverThresholdNS;
double timeOfArrivalNS;
long dcol;
long spix;
long pix;
unsigned short timeOverThreshold, timeOfArrival, spidrTime;
char chipnr, fineTimeOfArrival;
int frameNr;
int coarseTimeOfArrival;
int mode;
unsigned long Timer_LSB32 = 0;
unsigned long Timer_MSB16 = 0;
unsigned long numofTDC=0;

void processTDCPacket(unsigned long long datapacket, signalData &signalData) {
    // Logic for TDC packet

	// Unpack datapacket
	coarseTime = (datapacket >> 12) & 0xFFFFFFFF;                        
	tmpfine = (datapacket >> 5) & 0xF; 
	tmpfine = ((tmpfine - 1) << 9) / 12;
	trigTimeFine = (datapacket & 0x0000000000000E00) | (tmpfine & 0x00000000000001FF);

	// Set info in signalData 
    signalData.signalType = 1;
	signalData.xPixel = 0;
	signalData.yPixel = 0;
	signalData.ToaFinal = coarseTime*25*NANOSECS + trigTimeFine*timeUnit*NANOSECS;
	signalData.TotFinal = 0;

}

void processPixelPacket(unsigned long long datapacket, signalData &signalData) {
    // Logic for Pixel packet
    //cout << "Pixel packet" << endl;

    // Unpack datapacket
    spidrTime = (unsigned short)(datapacket & 0xffff);
    dcol = (datapacket & 0x0FE0000000000000L) >> 52;                                                                  
    spix = (datapacket & 0x001F800000000000L) >> 45;                                                                    
    pix = (datapacket & 0x0000700000000000L) >> 44;
    x = (int)(dcol + pix / 4);
    y = (int)(spix + (pix & 0x3));
    timeOfArrival = (unsigned short)((datapacket >> (16 + 14)) & 0x3fff);   
    timeOverThreshold = (unsigned short)((datapacket >> (16 + 4)) & 0x3ff);	
    fineTimeOfArrival = (unsigned char)((datapacket >> 16) & 0xf);
    coarseTimeOfArrival = (timeOfArrival << 4) | (~fineTimeOfArrival & 0xf);
    spidrTimens = spidrTime * 25.0 * 16384.0;
    //timeOfArrivalNS = timeOfArrival * 25.0;
    timeOverThresholdNS = timeOverThreshold * 25.0;	
    global_timestamp = spidrTimens + coarseTimeOfArrival * (25.0 / 16);
    
    // Set info in signalData 
    signalData.signalType = 2;
	signalData.xPixel = x;
	signalData.yPixel = y;
	signalData.ToaFinal = global_timestamp / 1E9;
	signalData.TotFinal = timeOverThresholdNS;
}

void processGlobalTimePacket(unsigned long long datapacket, signalData &signalData) {
    // Logic for Global time packet
   // cout << "Global time packet" << endl;

    if (((datapacket >> 56) & 0xF) == 0x4) {
        Timer_LSB32 = (datapacket >> 16) & 0xFFFFFFFF;
    }
    else if (((datapacket >> 56) & 0xF) == 0x5)
    {
        Timer_MSB16 = (datapacket >> 16) & 0xFFFF;
        unsigned long long int timemaster;
        timemaster = Timer_MSB16;
        timemaster = (timemaster << 32) & 0xFFFF00000000;
        timemaster = timemaster | Timer_LSB32;
        int diff = (spidrTime >> 14) - ((Timer_LSB32 >> 28) & 0x3);

        if ((spidrTime >> 14) == ((Timer_LSB32 >> 28) & 0x3)){ 						
        }
        else {                               
            Timer_MSB16 = Timer_MSB16 - diff;
        }  

        // Set info in signalData 
		signalData.signalType = 3;
		signalData.xPixel = 0;
		signalData.yPixel = 0;
		signalData.ToaFinal = timemaster * 25e-9;
		signalData.TotFinal = 0;
	}

}

tpx3FileDianostics unpackAndSortEntireTPX3File(configParameters configParams){
    
    // allocate container for diagnositcs while unpacking. 
    tpx3FileDianostics tpx3FileInfo; 

    // Some buffer timing info for diagnostics
    std::chrono::duration<double> bufferUnpackTime;
    //std::chrono::duration<double> bufferSortTime;
    //std::chrono::duration<double> bufferWriteTime;
    //std::chrono::duration<double> bufferClusterTime;

    // Open the TPX3File
    std::string fullTpx3Path = configParams.rawTPX3Folder + "/" + configParams.rawTPX3File;
    std::cout << "Opening TPX3 file at path: " << configParams.rawTPX3File << std::endl;
    std::ifstream tpxFile(fullTpx3Path, std::ios::binary);
    if (!tpxFile) {throw std::runtime_error("Unable to open input TPX3 file at path: " + fullTpx3Path);}

    // Using filesystem to get file size securely and handling large files
    try {
        tpx3FileInfo.filesize = std::filesystem::file_size(fullTpx3Path);
        std::cout << "File size: " << tpx3FileInfo.filesize << std::endl;
        tpx3FileInfo.numberOfDataPackets = tpx3FileInfo.filesize/8;
        std::cout << "Number of Data Packets: " << tpx3FileInfo.numberOfDataPackets << std::endl;

    } catch (std::filesystem::filesystem_error& e) {
        std::cerr << "Error getting file size: " << e.what() << '\n';
        return {}; // Return from your function or handle the error as needed
    }    

    // If writeRawSignals is true, attempt to create/open an output file for writing raw signals
    ofstream rawSignalsFile;
    if (configParams.writeRawSignals) {
        std::string fullOutputPath = configParams.outputFolder + "/" + configParams.runHandle +".rawsignals";
        rawSignalsFile.open(fullOutputPath, ios::out | ios::binary);
        if (!rawSignalsFile) {
            throw std::runtime_error("Unable to open output file!");
        }
    }

    // allocate an array for allSignalpackets, store all signals from the entire TPX3File, then close the TPX3 file.  
    uint64_t* allSignalpackets = new uint64_t [tpx3FileInfo.numberOfDataPackets];
    tpxFile.read((char*) allSignalpackets, tpx3FileInfo.filesize);
    tpxFile.close();

    
    // Initiate an array of signalData called signalDataArray that is the same size as bufferSize//8
    signalData* signalDataArray = new signalData[tpx3FileInfo.numberOfDataPackets];

    auto startUnpackTime = std::chrono::high_resolution_clock::now(); // Grab a START time for unpacking
    
    char* chunkHeader = new char[8];
    
    uint64_t i = 0; // Initialize the counter outside the while loop
    while (i < tpx3FileInfo.numberOfDataPackets) {
        
        // TODO: figure out if memcpy is needed. 
        memcpy(chunkHeader, &allSignalpackets[i], 8); 
        
        if (chunkHeader[0] == 'T' && chunkHeader[1] == 'P' && chunkHeader[2] == 'X') {

            // Grab the buffer (or chuck size)
            int bufferSize = ((0xff & chunkHeader[7]) << 8) | (0xff & chunkHeader[6]);

            // add to the number of buffers observed in the TPX3 file
            ++tpx3FileInfo.numberOfBuffers; 

            // TODO: figure out if these are needed. 
            chipnr = chunkHeader[4];    // Grab chip number
            mode = chunkHeader[5];      // Grab mode. 

            // calculate the number of data packets in the buffer
            int dataPacketsInBuffer = bufferSize/8;
        }
        i++; // Increment the counter at the end of the while loop
    }

    return tpx3FileInfo;
} 


/**
 * @brief Unpcaked and processes TPX3 file buffer by buffer.
 *
 * This function function takes a HERMES defined structure configParameters
 * and unpacks and processess signal data in a TPX3 file according to the 
 * how the control parameters (defined in configParameters) 
 *
 * @param configParams a configParameters structure that contains all the control parameters
 * for unpacking and processing TPX3 signals. 
 * @return tpx3FileInfo a tpx3FileDianostics structure that contains all the diagnostic containers for
 * unpacking and processing tpx3Files. 
 */
tpx3FileDianostics unpackandSortTPX3FileInSequentialBuffers(configParameters configParams){
    
    // initiate container for diagnositcs while unpacking. 
    tpx3FileDianostics tpx3FileInfo; 

    // Some buffer timing info for diagnostics
    std::chrono::duration<double> bufferUnpackTime;
    std::chrono::duration<double> bufferSortTime;
    std::chrono::duration<double> bufferWriteTime;
    std::chrono::duration<double> bufferClusterTime;

    // Open the TPX3File
    std::string fullTpx3Path = configParams.rawTPX3Folder + "/" + configParams.rawTPX3File;
    std::cout << "Opening TPX3 file at path: " << configParams.rawTPX3File << std::endl;
    std::ifstream tpxFile(fullTpx3Path, std::ios::binary);
    if (!tpxFile) {throw std::runtime_error("Unable to open input TPX3 file at path: " + fullTpx3Path);}
    
    // Determine length of file
    tpxFile.seekg(0, tpxFile.end);                  // Move the pointer to the end to find the file length
    tpx3FileInfo.filesize = tpxFile.tellg();        // Get the length of the file
    tpxFile.seekg(0, tpxFile.beg);                  // Move the pointer back to the beginning of the file for reading
    tpx3FileInfo.numberOfDataPackets = tpx3FileInfo.filesize/8;

    // If writeRawSignals is true, attempt to create/open an output file for writing raw signals
    ofstream rawSignalsFile;
    if (configParams.writeRawSignals) {
        std::string fullOutputPath = configParams.outputFolder + "/" + configParams.runHandle +".rawsignals";
        rawSignalsFile.open(fullOutputPath, ios::out | ios::binary);
        if (!rawSignalsFile) {
            throw std::runtime_error("Unable to open output file!");
        }
    }

    char* HeaderBuffer = new char[8];

    while(tpxFile.read(HeaderBuffer, 8)) {  

        // Read header buffer and check for TPX
        if (HeaderBuffer[0] == 'T' && HeaderBuffer[1] == 'P' && HeaderBuffer[2] == 'X') {
            int bufferSize = ((0xff & HeaderBuffer[7]) << 8) | (0xff & HeaderBuffer[6]);

            ++tpx3FileInfo.numberOfBuffers; 
            chipnr = HeaderBuffer[4];
            mode = HeaderBuffer[5];

            // calculate the number of data packets in the buffer
            int dataPacketsInBuffer = bufferSize/8;

            // Initiate an array of datapackets that is the same size as bufferSize//8
            uint64_t* datapackets = new uint64_t[dataPacketsInBuffer];

            // Initiate an array of signalData called signalDataArray that is the same size as bufferSize//8
            signalData* signalDataArray = new signalData[dataPacketsInBuffer];

            // Initiate an array of group ID that is the same size 
            int16_t* signalGroupID = new int16_t[dataPacketsInBuffer];
            memset(signalGroupID, 0, dataPacketsInBuffer * sizeof(int16_t));
            
            // Read the data buffer
            auto startUnpackTime = std::chrono::high_resolution_clock::now(); // Grab a START time for unpacking
            tpxFile.read((char*)datapackets, bufferSize);  

            // Process each data packet in buffer and update signalDataArray
            for (int j = 0; j < dataPacketsInBuffer; j++) {
                
                //int hdr = (int)(datapackets[j] >> 56);
                int packetHeader = datapackets[j] >> 60;

                switch (packetHeader){
                case 0x6:
                    //[TODO]: NEED to fix to account for 2 TDC signals!!
                    // maybe have processTDCPacket return bool (if 0 -> TDC1, if 1 ->TDC2)
                    processTDCPacket(datapackets[j], signalDataArray[j]);
                    ++tpx3FileInfo.numberOfTDC1s;  
                    break;
                case 0xb:
                    processPixelPacket(datapackets[j], signalDataArray[j]);
                    ++tpx3FileInfo.numberOfPixelHits;
                    break;
                case 0x4:
                    processGlobalTimePacket(datapackets[j], signalDataArray[j]);
                    ++tpx3FileInfo.numberOfGTS;
                    break;
                default:
                    break;
                }
            }

            // Grab stop time, calculate buffer unpacking duration, and append to total unpacking time.
            auto stopUnpackTime = std::chrono::high_resolution_clock::now(); // Grab a STOP time for unpacking
            bufferUnpackTime = stopUnpackTime - startUnpackTime;
            tpx3FileInfo.totalUnpackingTime = tpx3FileInfo.totalUnpackingTime + bufferUnpackTime.count();

            // If sortSingnals is true then sort!! 
            if (configParams.sortSignals){
                auto startSortTime = std::chrono::high_resolution_clock::now(); // Grab a start time for sorting
                
                // Print info depending on verbosity level
                if (configParams.verboseLevel>=3) {std::cout <<"Buffer "<< tpx3FileInfo.numberOfBuffers << ": Sorting raw signal data. " << std::endl;}

                // Sort the signalDataArray based on ToaFinal
                std::sort(signalDataArray, signalDataArray + dataPacketsInBuffer,[](const signalData &a, const signalData &b) -> bool {return a.ToaFinal < b.ToaFinal;});
                
                // Grab stop time, calculate buffer sort duration, and append to total sorting time.
                auto stopSortTime = std::chrono::high_resolution_clock::now();
                bufferSortTime = stopSortTime - startSortTime;
                tpx3FileInfo.totalSortingTime = tpx3FileInfo.totalSortingTime + bufferSortTime.count();
            }

            if (configParams.writeRawSignals){
                if (configParams.verboseLevel>=3) {std::cout <<"Buffer "<< tpx3FileInfo.numberOfBuffers << ": Writing out raw signal data. " << std::endl;}
                if(configParams.writeRawSignals){
                    auto startWriteTime = std::chrono::high_resolution_clock::now(); // Grab a start time for writing
                    rawSignalsFile.write(reinterpret_cast<char*>(signalDataArray), sizeof(signalData) * dataPacketsInBuffer);
                    // Grab stop time, calculate buffer sort duration, and append to total sorting time.
                    auto stopWriteTime = std::chrono::high_resolution_clock::now();
                    bufferWriteTime = stopWriteTime - startWriteTime;
                    tpx3FileInfo.totalWritingTime = tpx3FileInfo.totalWritingTime + bufferWriteTime.count();
                }
            }

            // If clustering is turned on then start grouping signals into photon events using Spatial-Temporal DBSCAN.
            if (configParams.clusterPixels){

                // Print message for each buffer if verboselevel = 3.
                if (configParams.verboseLevel>=3) {std::cout <<"Buffer "<< tpx3FileInfo.numberOfBuffers << ": Clustering pixels based on DBSCAN " << std::endl;}

                auto startClusterTime = std::chrono::high_resolution_clock::now();
                // sort pixels into clusters (photons) using sorting algorith. 
                ST_DBSCAN(configParams, signalDataArray, signalGroupID, dataPacketsInBuffer);
                auto stopClusterTime = std::chrono::high_resolution_clock::now();
                bufferClusterTime = stopClusterTime - startClusterTime;
                tpx3FileInfo.totalClusteringTime = tpx3FileInfo.totalClusteringTime + bufferClusterTime.count();
            }
            
            //
            if(configParams.maxBuffersToRead < 250 && configParams.maxBuffersToRead != 0){
                //printGroupIDs(numberOfBuffers,signalDataArray,signalGroupID,dataPacketsInBuffer);
            }

            delete[] datapackets;
            delete[] signalDataArray;
            delete[] signalGroupID;
        }
        
        // Increment and check buffer count against the limit if it's non-zero
        if (configParams.maxBuffersToRead > 0 && ++tpx3FileInfo.numberOfBuffers >= configParams.maxBuffersToRead) {
            if (configParams.verboseLevel>=2) {
                std::cout << "Limit of " << configParams.maxBuffersToRead << " buffers reached, stopping processing." << std::endl;
            }
            break; // Exit loop if limit reached
        }

        
    }

    // Close the TPX3File and print message depending on verbose level. 
    if(configParams.verboseLevel>=1){std::cout << "Closing TPX3 file: "<< configParams.rawTPX3File << std::endl;}
    tpxFile.close();

    // If writeRawSignals is true, then also close output file
    if(configParams.writeRawSignals){
        if(configParams.verboseLevel>=1){std::cout << "Closing raw output file" << std::endl;}
        rawSignalsFile.close();
    }

    return tpx3FileInfo;
}





















