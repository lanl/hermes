#include "structures.h"
#include "photonRecon.h"

#include <iostream>
#include <fstream> 

// Computes the spatial distance between two signal data points using Euclidean distance formula.
double spatialDistance(const signalData& p1, const signalData& p2) {
    return std::sqrt((p1.xPixel - p2.xPixel) * (p1.xPixel - p2.xPixel) +
                     (p1.yPixel - p2.yPixel) * (p1.yPixel - p2.yPixel));
}

// Computes the temporal distance between two signal data points.
double temporalDistance(const signalData& p1, const signalData& p2) {
    return std::abs(p1.ToaFinal - p2.ToaFinal);
}

// Checks if two signal data points are neighbors based on given spatial and temporal thresholds.
bool isNeighbor(const signalData& p1, const signalData& p2, double epsSpatial, double epsTemporal) {
    return spatialDistance(p1, p2) <= epsSpatial && temporalDistance(p1, p2) <= epsTemporal;
}

// Queries the dataset to find neighbors of a given signal data point based on spatial and temporal thresholds.
std::vector<size_t> regionQuery(signalData* signalDataArray, size_t pIndex, double epsSpatial, double epsTemporal, size_t dataPacketsInBuffer) {
    std::vector<size_t> neighbors;
    for (size_t i = 0; i < dataPacketsInBuffer; i++) {
        if (isNeighbor(signalDataArray[pIndex], signalDataArray[i], epsSpatial, epsTemporal)) {
            neighbors.push_back(i);
        }
    }
    return neighbors;
}

// Expands the cluster by adding reachable points based on spatial and temporal thresholds.
void expandCluster(configParameters config, signalData* signalDataArray, int16_t* signalGroupID, size_t pIndex, std::vector<size_t>& neighbors, int clusterId, size_t dataPacketsInBuffer, photonData& pd) {
    
    signalGroupID[pIndex] = clusterId;
    
    // Initialize weighted sums with  initial point in the calculations
    float weightedSumX = signalDataArray[pIndex].xPixel * signalDataArray[pIndex].TotFinal;    
    float weightedSumY =  signalDataArray[pIndex].yPixel * signalDataArray[pIndex].TotFinal;    
    double weightedSumToA = signalDataArray[pIndex].ToaFinal * signalDataArray[pIndex].TotFinal; 
    uint16_t totalToT = signalDataArray[pIndex].TotFinal;     

    // Loop through pixel neighbors, I think. 
    for (size_t i = 0; i < neighbors.size(); i++) {

        size_t qIndex = neighbors[i];
        if (signalGroupID[qIndex] == 0) {
            signalGroupID[qIndex] = clusterId;

            std::vector<size_t> qNeighbors = regionQuery(signalDataArray, qIndex, config.epsSpatial, config.epsTemporal, dataPacketsInBuffer);
            if (qNeighbors.size() >= static_cast<size_t>(config.minPts)) {
                neighbors.insert(neighbors.end(), qNeighbors.begin(), qNeighbors.end());
            }
        }
        // Assume ToT is accessible for each signal data point
        weightedSumX += signalDataArray[qIndex].xPixel * signalDataArray[qIndex].TotFinal;
        weightedSumY += signalDataArray[qIndex].yPixel * signalDataArray[qIndex].TotFinal;
        weightedSumToA += signalDataArray[qIndex].ToaFinal * signalDataArray[qIndex].TotFinal;
        totalToT +=  signalDataArray[qIndex].TotFinal;
    
    }
    // Calculate CoM based on distribution of x and y pixels and weighted by the tot and update photonData.
    pd.photon_x = weightedSumX/totalToT; // Calculated weighted center X
    pd.photon_y = weightedSumY/totalToT; // Calculated weighted center Y
    pd.photon_toa = weightedSumToA/totalToT;
    pd.integrated_tot = totalToT; // Total ToT for the cluster
    
}

// Implements the ST_DBSCAN clustering algorithm. 
// It segregates data points into clusters based on spatial and temporal closeness.
void ST_DBSCAN(configParameters config, signalData* signalDataArray, int16_t* signalGroupID, size_t dataPacketsInBuffer) {
    
    int clusterId = 0;
    // Initiate vector of photonData called photons.
    std::vector<photonData> photons;
    photonData singlePhoton;

    for (size_t i = 0; i < dataPacketsInBuffer; i++) {
        if (signalDataArray[i].signalType != 2) {
            signalGroupID[i] = -2; // signalType != 2
            continue; // Skip to the next iteration
        }

        if (signalGroupID[i] == 0) { // unclassified
            std::vector<size_t> neighbors = regionQuery(signalDataArray, i, config.epsSpatial, config.epsTemporal, dataPacketsInBuffer);
            if (neighbors.size() < static_cast<size_t>(config.minPts)) {
                signalGroupID[i] = -1; // noise
            } else {
                clusterId++;
                expandCluster(config, signalDataArray, signalGroupID, i, neighbors, clusterId, dataPacketsInBuffer, singlePhoton);
                photons.push_back(singlePhoton);
            }
        }
    }
    /* THis is wrong. I need to open the file before I enter the for loop that iterates through all the buffers. 
    if (config.writeOutPhotons == true){
        // Open a binary file for output
        //std::string fullOutputPath = config.outputFolder + "/" + config.runHandle +".photons";
        //std::ofstream outFile("photons.bin", std::ios::binary | std::ios::out);
        
        if (!outFile) {
            std::cerr << "Error opening file for writing." << std::endl;
            return; // Or handle the error as appropriate
        }

        // Iterate over the photons vector and write each photonData to the file
        for (const auto& photon : photons) {
            outFile.write(reinterpret_cast<const char*>(&photon), sizeof(photonData));
        }

        outFile.close(); // Close the file when done
    }*/
    

    // Print out diagnostics here...
    //std::cout << "Total number of photons: " << photons.size() << std::endl;
    //for (size_t i = 0; i < photons.size(); ++i) {
    //    std::cout << "x = " << photons[i].photon_x << ", "
    //              << "y = " << photons[i].photon_y << ", "
    //              << "ToA = " << photons[i].photon_toa << ", "
    //              << "ToT = " << photons[i].integrated_tot << std::endl;
    //}

}