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
unsigned short timeOverThreshold, timeOfArrival;
char chipnr, fineTimeOfArrival;
int frameNr;
int coarseTimeOfArrival;
int mode;

// Needed for global time stamps
uint32_t timeStampLow_25nsClock = 0;    
uint16_t timeStampHigh_107sClock = 0;
uint16_t spidrTime = 0;
unsigned long Timer_LSB32 = 0;
unsigned long Timer_MSB16 = 0;
unsigned long numofTDC=0;

// Some buffer timing info for diagnostics
std::chrono::duration<double> bufferUnpackTime;
std::chrono::duration<double> bufferSortTime;
std::chrono::duration<double> bufferWriteTime;
std::chrono::duration<double> bufferClusterTime;

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

// TODO: I need to rewrite this for clarity. 
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

// TODO: I need to rewrite this for clarity. Also need to figure out logic for time stamps. 
void processGlobalTimePacket(uint64_t datapacket, signalData &signalData) {
    // Logic for Global time packet
    uint8_t timeType = static_cast<uint8_t>((datapacket >> 56) & 0xFF);                        

    // Time Low packet
    if (timeType == 0x44) {
        timeStampLow_25nsClock = static_cast<uint32_t>((datapacket >> 16) & 0xFFFFFFFF);    // Extract 32-bit timestamp
        spidrTime = static_cast<uint16_t>(datapacket & 0xFFFF);                             // Extract 16-bit SPIDR time
        global_timestamp = timeStampLow_25nsClock*25E-9;

    // Time High packet
    } else if (timeType == 0x45) { 
        timeStampHigh_107sClock = static_cast<uint16_t>((datapacket >> 16) & 0xFFFF);       // Extract 16-bit timestamp
        spidrTime = static_cast<uint16_t>(datapacket & 0xFFFF);                             // Extract 16-bit SPIDR time
        global_timestamp = timeStampHigh_107sClock*107.374182;
    }

    // Set info in signalData 
    signalData.signalType = 3;
    signalData.xPixel = 0;
    signalData.yPixel = 0;
    signalData.ToaFinal = global_timestamp;
    signalData.TotFinal = spidrTime;
	
}

tpx3FileDianostics unpackAndSortEntireTPX3File(configParameters configParams){
    
    // allocate container for diagnositcs while unpacking. 
    tpx3FileDianostics tpx3FileInfo; 

    // Open the TPX3File
    std::string fullTpx3Path = configParams.rawTPX3Folder + "/" + configParams.rawTPX3File;
    std::cout << "Opening TPX3 file at path: " << configParams.rawTPX3File << std::endl;
    std::ifstream tpxFile(fullTpx3Path, std::ios::binary);
    if (!tpxFile) {throw std::runtime_error("Unable to open input TPX3 file at path: " + fullTpx3Path);}

    // Using filesystem to get file size securely and handling large files
    try {
        tpx3FileInfo.filesize = std::filesystem::file_size(fullTpx3Path);
        tpx3FileInfo.numberOfDataPackets = tpx3FileInfo.filesize/8;
        if (configParams.verboseLevel>=2) {
            std::cout << "File size: " << tpx3FileInfo.filesize << std::endl;
            std::cout << "Number of Data Packets: " << tpx3FileInfo.numberOfDataPackets << std::endl;
        }

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

    // Initiate an array of group ID that is the same size 
    int32_t* signalGroupID = new int32_t[tpx3FileInfo.numberOfDataPackets];
    memset(signalGroupID, 0, tpx3FileInfo.numberOfDataPackets * sizeof(int32_t));

    // Grab a START time for unpacking
    auto startUnpackTime = std::chrono::high_resolution_clock::now(); 
    
    // Convert "TPX3" string to uint32_t in little endian format
    const char tpx3SignatureStr[] = "TPX3";
    uint32_t tpx3Signature;
    memcpy(&tpx3Signature, tpx3SignatureStr, sizeof(uint32_t));

    uint64_t currentPacket = 0; // Initialize the counter outside the while loop
    
    // Continue to loop through datapacket array until you hit the numberOfDataPackets 
    while (currentPacket < tpx3FileInfo.numberOfDataPackets) {
        // Grab last 32 bits of current packt
        uint32_t tpx3flag = static_cast<uint32_t>(allSignalpackets[currentPacket]);

        // If tpx3flag matches the "TPX3" then you found a chuck header
        if (tpx3flag == tpx3Signature) {

            // Start processing the chuck header to get size and number of data packets in chuck. 
            uint16_t chunkSize = static_cast<uint16_t>((allSignalpackets[currentPacket] >> 48) & 0xFFFF);
            uint16_t numPacketsInChunk = chunkSize / 8;

            // Iterate through each data packet in the current chunk
            for (uint16_t chunkPacketIndex = 1; chunkPacketIndex <= numPacketsInChunk; ++chunkPacketIndex) {
                // Calculate the actual index of the current data packet within the entire array
                uint64_t actualPacketIndex = currentPacket + chunkPacketIndex;

                // Safeguard against going beyond the allocated memory
                if (actualPacketIndex >= tpx3FileInfo.numberOfDataPackets) break;

                // Extract the packet type from the most significant nibble
                uint8_t packetType = static_cast<uint8_t>((allSignalpackets[actualPacketIndex] >> 60) & 0xF);

                switch (packetType) {
                    case 0xA: // Integrated ToT mode
                        // Process Integrated ToT mode packet
                        std::cout << "Integrated ToT mode packet detected." << std::endl;
                        break;
                    case 0xB: // Time of Arrival mode
                        processPixelPacket(allSignalpackets[actualPacketIndex], signalDataArray[actualPacketIndex]);
                        tpx3FileInfo.numberOfPixelHits++;
                        break;
                    case 0x6: // TDC data packets
                        processTDCPacket(allSignalpackets[actualPacketIndex], signalDataArray[actualPacketIndex]);
                        tpx3FileInfo.numberOfTDC1s++;
                        break;
                    case 0x4: // Global time data packets
                        {
                            processGlobalTimePacket(allSignalpackets[actualPacketIndex], signalDataArray[actualPacketIndex]);
                            tpx3FileInfo.numberOfGTS++;
                            break;
                        }
                    case 0x5: { // SPIDR control packets, note the added braces to introduce a new block scope
                        uint8_t subType = static_cast<uint8_t>((allSignalpackets[actualPacketIndex] >> 56) & 0xF);
                        switch (subType) {
                            case 0xF:
                                if(configParams.verboseLevel >= 4){std::cout << "Open shutter packet detected." << std::endl;}
                                break;
                            case 0xA:
                                if(configParams.verboseLevel >= 4){std::cout << "Close shutter packet detected." << std::endl;}
                                break;
                            case 0xC:
                                if(configParams.verboseLevel >= 4){std::cout << "Heartbeat packet detected." << std::endl;}
                                break;
                            default:
                                if(configParams.verboseLevel >= 4){std::cout << "Unknown SPIDR control packet subtype detected." << std::endl;}
                                break;
                        }
                        break;
                    }
                    case 0x7: { // TPX3 control packets, again note the added braces
                        uint8_t controlType = static_cast<uint8_t>((allSignalpackets[actualPacketIndex] >> 48) & 0xFF);
                        if (controlType ==0xA0) {
                            if(configParams.verboseLevel >= 4){
                                std::cout << "End of sequential readout detected with "<< std::hex << "packetType: 0x" << +packetType << "\t"<< "countrolType: 0x" << +controlType << std::endl;
                            }

                        } else if (controlType == 0xB0) {
                            if(configParams.verboseLevel >= 4){
                                std::cout << "End of data driven readout detected with "<< std::hex << "packetType: 0x" << +packetType << "\t"<< "countrolType: 0x" << +controlType << std::endl;
                            }
                        } else {
                            if(configParams.verboseLevel >= 4){
                            // Print out the hex value of controlType
                                std::cout << "Unkown TPX3 control packets detected with "
                                << "packetType: 0x"<< std::hex << +packetType << "\t"
                                << "countrolType: 0x" << controlType << std::endl;
                            }
                        }
                        tpx3FileInfo.numberOfTXP3Controls++;
                        break;
                    }
                }
            }

            // Move to the next chunk by skipping over the packets in the current chunk
            currentPacket += numPacketsInChunk - 1; // -1 because the loop increment will add 1 more
            tpx3FileInfo.numberOfBuffers++;
        } 
        currentPacket++;
    }

    // Grab stop time, calculate buffer unpacking duration, and append to total unpacking time.
    auto stopUnpackTime = std::chrono::high_resolution_clock::now(); // Grab a STOP time for unpacking
    bufferUnpackTime = stopUnpackTime - startUnpackTime;
    tpx3FileInfo.totalUnpackingTime = tpx3FileInfo.totalUnpackingTime + bufferUnpackTime.count();

    // If sortSingnals is true then sort!! 
    if (configParams.sortSignals){
        auto startSortTime = std::chrono::high_resolution_clock::now(); // Grab a start time for sorting
        
        // Print info depending on verbosity level
        if (configParams.verboseLevel>=3) {std::cout <<"Sorting raw signal data. " << std::endl;}

        // Sort the signalDataArray based on ToaFinal
        std::sort(signalDataArray, signalDataArray + tpx3FileInfo.numberOfDataPackets,[](const signalData &a, const signalData &b) -> bool {return a.ToaFinal < b.ToaFinal;});
        
        // Grab stop time, calculate buffer sort duration, and append to total sorting time.
        auto stopSortTime = std::chrono::high_resolution_clock::now();
        bufferSortTime = stopSortTime - startSortTime;
        tpx3FileInfo.totalSortingTime = tpx3FileInfo.totalSortingTime + bufferSortTime.count();
    }

    // TODO: Check if this is correct!!
    if (configParams.writeRawSignals){
        if (configParams.verboseLevel>=3) {std::cout <<"Writing out raw signal data. " << std::endl;}
        if(configParams.writeRawSignals){
            auto startWriteTime = std::chrono::high_resolution_clock::now(); // Grab a start time for writing
            rawSignalsFile.write(reinterpret_cast<char*>(signalDataArray), sizeof(signalData) * tpx3FileInfo.numberOfDataPackets);
            // Grab stop time, calculate buffer sort duration, and append to total sorting time.
            auto stopWriteTime = std::chrono::high_resolution_clock::now();
            bufferWriteTime = stopWriteTime - startWriteTime;
            tpx3FileInfo.totalWritingTime = tpx3FileInfo.totalWritingTime + bufferWriteTime.count();
        }
    }
    // If clustering is turned on then start grouping signals into photon events using Spatial-Temporal DBSCAN.
    if (configParams.clusterPixels){

        // Print message for each buffer if verboselevel = 3.
        if (configParams.verboseLevel>=3) {std::cout <<"Clustering pixels based on DBSCAN " << std::endl;}

        auto startClusterTime = std::chrono::high_resolution_clock::now();
        // sort pixels into clusters (photons) using sorting algorith. 
        ST_DBSCAN(configParams, signalDataArray, signalGroupID, tpx3FileInfo.numberOfDataPackets);
        auto stopClusterTime = std::chrono::high_resolution_clock::now();
        bufferClusterTime = stopClusterTime - startClusterTime;
        tpx3FileInfo.totalClusteringTime = tpx3FileInfo.totalClusteringTime + bufferClusterTime.count();
    }



    delete[] allSignalpackets; // Don't forget to free the allocated memory
    delete[] signalDataArray;  // Assuming you're done with signalDataArray

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
            int32_t* signalGroupID = new int32_t[dataPacketsInBuffer];
            memset(signalGroupID, 0, dataPacketsInBuffer * sizeof(int32_t));
            
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





















