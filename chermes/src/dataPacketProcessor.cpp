// dataPacketProcessor.cpp
#include <stdio.h>
#include <iostream>

#include "dataPacketProcessor.h"
#include "structures.h"

using namespace std;

const double NANOSECS = 1E-9;


// Some global variables, constants, and containers
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





















