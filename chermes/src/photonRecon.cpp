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

/**
 * @brief Queries the dataset to find neighbors of a specific signal data point within a defined range based on spatial and temporal thresholds.
 *
 * This function searches for neighboring points around a specified 'homeIndex' in the 'signalDataArray'. The search is confined within
 * a range of [-500, +500] indices from 'homeIndex', adjusted to stay within the bounds of the array. Neighbors are determined based on
 * the spatial and temporal distance thresholds specified in 'configParams'.
 *
 * @param configParams Configuration parameters including spatial (epsSpatial) and temporal (epsTemporal) thresholds for determining neighbors.
 * @param tpx3FileInfo An object containing file diagnostics, including the total number of data packets (numberOfDataPackets) to ensure search indices remain valid.
 * @param signalDataArray A pointer to the array of signal data points, each of which includes spatial coordinates, time of arrival, etc.
 * @param homeIndex The index of the signal data point in 'signalDataArray' for which neighbors are being searched.
 *
 * @return std::vector<size_t> A vector containing the indices of all neighboring data points within the specified spatial and temporal thresholds.
 *
 * Example usage:
 * auto neighbors = regionQuery(config, fileInfo, signalData, 1000);
 * This example searches for neighbors of the data point at index 1000 within the signalData array, using thresholds defined in 'config'.
 */
std::vector<size_t> regionQuery(configParameters configParams, tpx3FileDiagnostics& tpx3FileInfo, signalData* signalDataArray, const size_t homeIndex) {
    
    // Calculate start and end indices to search within, ensuring they stay within bounds of the array
    size_t startIndex = homeIndex > 500 ? homeIndex - 500 : 0;
    size_t endIndex = homeIndex + 500 < tpx3FileInfo.numberOfDataPackets ? homeIndex + 500 : tpx3FileInfo.numberOfDataPackets - 1;
    
    std::vector<size_t> neighbors;
    for (size_t i = startIndex; i <= endIndex; i++) {
        if (isNeighbor(signalDataArray[homeIndex], signalDataArray[i], configParams.epsSpatial, configParams.epsTemporal)) {
            neighbors.push_back(i);
        }
    }
    return neighbors;
}

/*
// Expands the cluster by adding reachable points based on spatial and temporal thresholds.
void expandCluster(configParameters config, signalData* signalDataArray, size_t pIndex, std::vector<size_t>& neighbors, int clusterId, size_t dataPacketsInBuffer, photonData& pd) {
    
    signalDataArray->groupID[pIndex] = clusterId;
    
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

            // --->>> Here I need to make a sub array of signalDataArray to reduce the size of the region that I am querying. 
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
*/

// Implements the ST_DBSCAN clustering algorithm. 
// It segregates data points into clusters based on spatial and temporal closeness.
void ST_DBSCAN(configParameters configParams, tpx3FileDiagnostics& tpx3FileInfo, signalData* signalDataArray) {
    
    size_t clusterId = 0;
    // Initiate vector of photonData called photons.
    std::vector<photonData> photons;
    photonData singlePhoton;

    for (size_t i = 0; i < tpx3FileInfo.numberOfDataPackets; i++) {
        if (signalDataArray[i].signalType != 2) {
            signalDataArray[i].groupID = -2; // signalType != 2
            continue; // Skip to the next iteration
        }

        if (signalDataArray[i].groupID == 0) { // unclassified
            std::vector<size_t> neighbors = regionQuery(configParams, tpx3FileInfo, signalDataArray, i);
            
            if (neighbors.size() < static_cast<size_t>(configParams.minPts)) {signalDataArray[i].groupID = -1;} // Noise or gammas 
            else {
                clusterId++;
                //expandCluster(configParams, signalDataArray, i, neighbors, clusterId, tpx3FileInfo.numberOfDataPackets, singlePhoton);
                //photons.push_back(singlePhoton);
            }
        }
    }
}