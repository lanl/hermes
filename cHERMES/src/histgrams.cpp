#include "structures.h"
#include "histograms.h"

// Definition of histograms
TH1F *pixelToA_1D = new TH1F("toaFinalHist", "Time of Arrival - Pixels", 1000000, 0.002, 0.02);
TH1F *tdcToA_1D = new TH1F("tdcFinalHist", "Time of Arrival - TDCs", 1000000, 0.002, 0.02);
TH1F *gtsToA_1D = new TH1F("gtsFinalHist", "Time of Arrival - Global Timestamps", 100000, 0.002, 0.009);
TH2C *pixelHits_2D = new TH2C("pixelHits","Postion of Pixel Hits",256, 0, 256, 256, 0, 256);


void fillRawTDCHistogram(signalData &data){
	tdcToA_1D->Fill(data.ToaFinal);
}

void fillRawPixelHistogram(signalData &data){
	pixelToA_1D->Fill(data.ToaFinal);
	pixelHits_2D->Fill(data.yPixel,data.xPixel);
}

void fillRawGTSHistogram(signalData &data){
	gtsToA_1D->Fill(data.ToaFinal);
}

void fillRawHistograms(int dataPacketsInBuffer, signalData* signalDataArray) {
    for (int i = 0; i < dataPacketsInBuffer; ++i) {
		const signalData &data = signalDataArray[i];
		if (data.signalType == 1) {
			tdcToA_1D->Fill(data.ToaFinal);
		} else if (data.signalType == 2){
			pixelToA_1D->Fill(data.ToaFinal);
			pixelHits_2D->Fill(data.yPixel,data.xPixel);
		} else if (data.signalType == 3){
			gtsToA_1D->Fill(data.ToaFinal);
		}
	}
}

void writeHistograms() {
    
    // Write all 1D hists
    pixelToA_1D->Write();
    tdcToA_1D->Write();
    gtsToA_1D->Write();

    //Write 2D hists
    pixelHits_2D->Write();

}