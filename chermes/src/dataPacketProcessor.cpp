// dataPacketProcessor.cpp
#include <filesystem>
#include <algorithm>


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


/**
 * @brief Opens a TPX3 file, calculates file size and number of data packets, updates diagnostic information, and returns an ifstream object.
 *
 * This function constructs the full path to a TPX3 file using the `configParameters` structure provided in `config`. 
 * It attempts to open the specified TPX3 file in binary mode. Upon successfully opening the file, it utilizes the filesystem 
 * library to securely obtain the file's size. It then calculates the number of data packets in the TPX3 file based on 
 * the file size and updates the `tpx3FileDiagnostics` structure `tpx3FileInfo` with this information. The function outputs 
 * relevant information to the console, depending on the verbosity level specified in `config`. If the file cannot be opened 
 * or if there is an error obtaining the file size, the function handles these errors appropriately by either throwing a runtime_error 
 * exception or closing the file and rethrowing the exception to ensure clean error handling and resource management. 
 * Finally, it returns the ifstream object associated with the opened file, allowing for further processing.
 *
 * @param config       The `configParameters` structure that holds control parameters for the operation, 
 * including the directory and filename of the TPX3 file, and the verbosity level for logging.
 * @param tpx3FileInfo A reference to a `tpx3FileDiagnostics` structure that will be updated with the file 
 * size and number of data packets. This structure should be used later for diagnostics and processing information.
 * @return std::ifstream An ifstream object associated with the opened TPX3 file. This object is used for subsequent file reading operations.
 * @throws std::runtime_error if the TPX3 file cannot be opened or if there is an error obtaining the file size.
 */
std::ifstream openTPX3File(const configParameters& config, tpx3FileDiagnostics& tpx3FileInfo) {
    // Construct the full path to the TPX3 file
    std::string fullTpx3Path = config.rawTPX3Folder + "/" + config.rawTPX3File;
    std::cout << "Opening TPX3 file at path: " << fullTpx3Path << std::endl;
    
    // Attempt to open the TPX3 file
    std::ifstream tpx3File(fullTpx3Path, std::ios::binary);
    
    // Check if the file stream is in a good state (i.e., the file has been successfully opened)
    if (!tpx3File) {
        // If the file cannot be opened, throw an exception
        throw std::runtime_error("Unable to open input TPX3 file at path: " + fullTpx3Path);
    }

    // Using filesystem to get file size securely and handling large files
    try {
        tpx3FileInfo.filesize = std::filesystem::file_size(fullTpx3Path);
        tpx3FileInfo.numberOfDataPackets = tpx3FileInfo.filesize / 8;
        if (config.verboseLevel >= 2) {
            std::cout << "File size: " << tpx3FileInfo.filesize << " bytes" << std::endl;
            std::cout << "Number of Data Packets: " << tpx3FileInfo.numberOfDataPackets << std::endl;
        }
    } catch (std::filesystem::filesystem_error& e) {
        std::cerr << "Error getting file size: " << e.what() << '\n';
        // Properly handle the error, such as by closing the file and rethrowing the exception
        tpx3File.close();
        throw;
    }
    
    return tpx3File; // Return the ifstream object associated with the opened file
}

/**
 * @brief Opens an output file for raw signals based on configuration parameters.
 *
 * If the writeRawSignals flag within the configParameters structure is set to true, this function constructs
 * the full path to the output file using the outputFolder and runHandle specified in config. It then attempts
 * to open this file for binary output. If the file is successfully opened, it returns an ofstream object
 * associated with the file. If writeRawSignals is false or the file cannot be opened, it handles the
 * situation appropriately, either by not attempting to open the file or by throwing an exception.
 *
 * @param config         The configParameters structure containing the output file configuration, including
 *                       the output folder, run handle, and the flag indicating whether to write raw signals.
 * @return std::ofstream An ofstream object for the output file. If writeRawSignals is false, this ofstream
 *                       may not be opened (check with is_open() before using).
 * @throws std::runtime_error If the file cannot be opened when writeRawSignals is true.
 */
std::ofstream openRawSignalsOutputFile(const configParameters& config) {
    std::ofstream rawSignalsFile;

    // Check if writing raw signals is enabled
    if (config.writeRawSignals) {
        // Construct the full path to the output file
        std::string fullOutputPath = config.outputFolder + "/" + config.runHandle + ".rawsignals";
        std::cout << "Opening output file for raw signals at path: " << fullOutputPath << std::endl;

        // Attempt to open the output file
        rawSignalsFile.open(fullOutputPath, std::ios::out | std::ios::binary);

        // Check if the file stream is in a good state (i.e., the file has been successfully opened)
        if (!rawSignalsFile) {
            // If the file cannot be opened, throw an exception
            throw std::runtime_error("Unable to open output file for raw signals at path: " + fullOutputPath);
        }
    } else {
        std::cout << "Writing raw signals is disabled." << std::endl;
    }

    // Return the ofstream object associated with the opened file
    // Note: If writing is disabled, this returns an unopened ofstream, which the caller needs to check
    return rawSignalsFile;
}

/**
 * @brief Takes a TDC data packet from a tpx3 file, processes it, and updates the corresponding signal data structure. 
 *
 * This function takes the data packet pass through unsigned long long datapacket and processes the timing of 
 * the TDC trigger. It then update the corresponding signalData structure, which is passed by refference. 
 * 
 * @param datapacket     64 btye data packet that contains raw timing info
 * @param signalData    HERMES defined structure that contain raw data info from data packets.
 * @return nothing
 */
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


/**
 * @brief Takes a Pixel data packet from a tpx3 file, processes it, and updates the corresponding signal data structure. 
 *
 * This function takes the data packet pass through unsigned long long datapacket and processes the timing and position of 
 * the Pixel hit. It then update the corresponding signalData structure, which is passed by refference. 
 * 
 * TODO: I need to rewrite this for clarity. 
 * 
 * @param datapacket     64 btye data packet that contains raw timing info
 * @param signalData    HERMES defined structure that contain raw data info from data packets.
 * @return nothing
 */ 
void processPixelPacket(unsigned long long datapacket, signalData &signalData) {
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


/**
 * @brief Takes a Global Time Stamp data packet from a tpx3 file, processes it, 
 * and updates the corresponding signal data structure. 
 *
 * This function takes the data packet pass through unsigned long long datapacket and processes the timing of 
 * the Global Time Stamp hit. It then update the corresponding signalData structure, which is passed by refference. 
 * 
 * TODO: I need to rewrite this for clarity. Also need to figure out logic for time stamps.  
 * 
 * @param datapacket     64 btye data packet that contains raw timing info
 * @param signalData    HERMES defined structure that contain raw data info from data packets.
 * @return nothing
 */ 
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


void processDataPackets(const configParameters& config, tpx3FileDiagnostics& tpx3FileInfo, const uint64_t* packets, signalData* signalDataArray, size_t numPackets) {
    for (size_t i = 0; i < numPackets; ++i) {
        // Extract packet type or other identifying information
        uint8_t packetType = extractPacketType(packets[i]);

        switch (packetType) {
            case PACKET_TYPE_PIXEL_HIT:
                // Process a pixel hit packet
                processPixelPacket(packets[i], signalDataArray[i], tpx3FileInfo);
                break;
            case PACKET_TYPE_TDC_HIT:
                // Process a TDC hit packet
                processTDCPacket(packets[i], signalDataArray[i], tpx3FileInfo);
                break;
            // Add cases for other packet types as necessary
            default:
                std::cerr << "Unknown packet type encountered: " << static_cast<unsigned>(packetType) << std::endl;
                // Handle unknown packet type, if necessary
                break;
        }
    }

    // Optionally, update tpx3FileInfo based on processed data
    updateDiagnostics(tpx3FileInfo, ...);
}


/**
 * @brief Takes a Global Time Stamp data packet from a tpx3 file, processes it, 
 * and updates the corresponding signal data structure. 
 *
 * This function takes the data packet pass through unsigned long long datapacket and processes the timing of 
 * the Global Time Stamp hit. It then update the corresponding signalData structure, which is passed by refference. 
 * 
 * TODO: I need to rewrite this for clarity. Also need to figure out logic for time stamps.  
 * 
 * @param datapacket     64 btye data packet that contains raw timing info
 * @param signalData    HERMES defined structure that contain raw data info from data packets.
 * @return nothing
 */ 
tpx3FileDiagnostics unpackAndSortTPX3File(configParameters configParams){
    
    std::ifstream tpx3File;             // initiate a input filestream for reading in TPX3file
    std::ofstream rawSignalsFile;       // initiate a output filestream for writing raw data
    tpx3FileDiagnostics tpx3FileInfo;    // initiate container for diagnositcs while unpacking.

    // Open the TPX3File
    try {tpx3File = openTPX3File(configParams,tpx3FileInfo);} 
    catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return {};  // Handle the error as needed, possibly returning an error code or an empty object
    }  

    // If writeRawSignals is true, Attempt to open the output file for raw signals, if configured to do so
    try {rawSignalsFile = openRawSignalsOutputFile(configParams);} 
    catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return {}; // Handle the erroras needed, possibly returning an error code or an empty object
    }

    // allocate an array for allTpx3Datapackets, store all signals from the entire TPX3File, then close the TPX3 file.  
    uint64_t* allTpx3Datapackets = new uint64_t [tpx3FileInfo.numberOfDataPackets];
    tpx3File.read((char*) allTpx3Datapackets, tpx3FileInfo.filesize);
    tpx3File.close();

    // Initiate an array of signalData called signalDataArray that is the same size as bufferSize//8
    signalData* signalDataArray = new signalData[tpx3FileInfo.numberOfDataPackets];

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
        uint32_t tpx3flag = static_cast<uint32_t>(allTpx3Datapackets[currentPacket]);

        // If tpx3flag matches the "TPX3" then you found a chuck header
        if (tpx3flag == tpx3Signature) {

            // Start processing the chuck header to get size and number of data packets in chuck. 
            uint16_t chunkSize = static_cast<uint16_t>((allTpx3Datapackets[currentPacket] >> 48) & 0xFFFF);
            uint16_t numPacketsInChunk = chunkSize / 8;

            // Iterate through each data packet in the current chunk
            for (uint16_t chunkPacketIndex = 1; chunkPacketIndex <= numPacketsInChunk; ++chunkPacketIndex) {
                // Calculate the actual index of the current data packet within the entire array
                uint64_t actualPacketIndex = currentPacket + chunkPacketIndex;

                // Safeguard against going beyond the allocated memory
                if (actualPacketIndex >= tpx3FileInfo.numberOfDataPackets) break;

                // Extract the packet type from the most significant nibble
                uint8_t packetType = static_cast<uint8_t>((allTpx3Datapackets[actualPacketIndex] >> 60) & 0xF);

                switch (packetType) {
                    case 0xA: // Integrated ToT mode
                        // Process Integrated ToT mode packet
                        std::cout << "Integrated ToT mode packet detected." << std::endl;
                        break;
                    case 0xB: // Time of Arrival mode
                        processPixelPacket(allTpx3Datapackets[actualPacketIndex], signalDataArray[actualPacketIndex]);
                        tpx3FileInfo.numberOfPixelHits++;
                        break;
                    case 0x6: // TDC data packets
                        processTDCPacket(allTpx3Datapackets[actualPacketIndex], signalDataArray[actualPacketIndex]);
                        tpx3FileInfo.numberOfTDC1s++;
                        break;
                    case 0x4: // Global time data packets
                        {
                            processGlobalTimePacket(allTpx3Datapackets[actualPacketIndex], signalDataArray[actualPacketIndex]);
                            tpx3FileInfo.numberOfGTS++;
                            break;
                        }
                    case 0x5: { // SPIDR control packets, note the added braces to introduce a new block scope
                        uint8_t subType = static_cast<uint8_t>((allTpx3Datapackets[actualPacketIndex] >> 56) & 0xF);
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
                        uint8_t controlType = static_cast<uint8_t>((allTpx3Datapackets[actualPacketIndex] >> 48) & 0xFF);
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



    delete[] allTpx3Datapackets; // Don't forget to free the allocated memory
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
/*
tpx3FileDiagnostics unpackandSortTPX3FileInSequentialBuffers(configParameters configParams){
     
    std::ifstream tpx3File;             // initiate a input filestream for reading in TPX3file
    std::ofstream rawSignalsFile;       // initiate a output filestream for writing raw data
    tpx3FileDiagnostics tpx3FileInfo;    // initiate container for diagnositcs while unpacking.

    // Open the TPX3File
    try {tpx3File = openTPX3File(configParams,tpx3FileInfo);} 
    catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return {};  // Handle the error as needed, possibly returning an error code or an empty object
    }  

    // If writeRawSignals is true, Attempt to open the output file for raw signals, if configured to do so
    try {rawSignalsFile = openRawSignalsOutputFile(configParams);} 
    catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return {}; // Handle the erroras needed, possibly returning an error code or an empty object
    }

    char* HeaderBuffer = new char[8];

    while(tpx3File.read(HeaderBuffer, 8)) {  

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
            tpx3File.read((char*)datapackets, bufferSize);  

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
    tpx3File.close();

    // If writeRawSignals is true, then also close output file
    if(configParams.writeRawSignals){
        if(configParams.verboseLevel>=1){std::cout << "Closing raw output file" << std::endl;}
        rawSignalsFile.close();
    }

    return tpx3FileInfo;
}


*/


















