

#include "structures.h"
#include "photonRecon.h"

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
void expandCluster(signalData* signalDataArray, int16_t* signalGroupID, size_t pIndex, std::vector<size_t>& neighbors, int clusterId, double epsSpatial, double epsTemporal, int minPts, size_t dataPacketsInBuffer, std::unordered_map<int, clusterInfo>& clusterDetails) {
    signalGroupID[pIndex] = clusterId;

    double minToa = signalDataArray[pIndex].ToaFinal;
    double maxToa = signalDataArray[pIndex].ToaFinal;

    for (size_t i = 0; i < neighbors.size(); i++) {
        size_t qIndex = neighbors[i];
        if (signalGroupID[qIndex] == 0) {
            signalGroupID[qIndex] = clusterId;

            // Update the min and max ToaFinal for the current cluster
            minToa = std::min(minToa, signalDataArray[qIndex].ToaFinal);
            maxToa = std::max(maxToa, signalDataArray[qIndex].ToaFinal);

            std::vector<size_t> qNeighbors = regionQuery(signalDataArray, qIndex, epsSpatial, epsTemporal, dataPacketsInBuffer);
            if (qNeighbors.size() >= static_cast<size_t>(minPts)) {
                neighbors.insert(neighbors.end(), qNeighbors.begin(), qNeighbors.end());
            }
        }
    }

    double timeDuration = maxToa-minToa;    

    // Store cluster details (can also add more information if needed in the future)
    //clusterDetails[clusterId] = {static_cast<int>(neighbors.size() + 1), timeDuration};
    //clusterDetails[clusterId] = std::make_pair(static_cast<int>(neighbors.size() + 1), timeDuration);

}

// Implements the ST_DBSCAN clustering algorithm. 
// It segregates data points into clusters based on spatial and temporal closeness.
void ST_DBSCAN(signalData* signalDataArray, int16_t* signalGroupID, double epsSpatial, double epsTemporal, int minPts, size_t dataPacketsInBuffer) {
    
    int clusterId = 0;
    std::unordered_map<int, clusterInfo> clusterDetails;

    for (size_t i = 0; i < dataPacketsInBuffer; i++) {
        if (signalDataArray[i].signalType != 2) {
            signalGroupID[i] = -2; // signalType != 2
            continue; // Skip to the next iteration
        }

        if (signalGroupID[i] == 0) { // unclassified
            std::vector<size_t> neighbors = regionQuery(signalDataArray, i, epsSpatial, epsTemporal, dataPacketsInBuffer);
            if (neighbors.size() < static_cast<size_t>(minPts)) {
                signalGroupID[i] = -1; // noise
            } else {
                clusterId++;
                expandCluster(signalDataArray, signalGroupID, i, neighbors, clusterId, epsSpatial, epsTemporal, minPts, dataPacketsInBuffer, clusterDetails);
                //clusterMultiplicity[clusterId] = neighbors.size() + 1; 
            }
        }
    }
}





