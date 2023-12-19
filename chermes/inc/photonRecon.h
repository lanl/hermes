// photonRecon.h

#ifndef PHOTONRECON_H
#define PHOTONRECON_H

#include <stdint.h>
#include <cstddef>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <cmath>


// User defined libraries
#include "structures.h"


double spatialDistance(const signalData& p1, const signalData& p2);
double temporalDistance(const signalData& p1, const signalData& p2);
bool isNeighbor(const signalData& p1, const signalData& p2, double epsSpatial, double epsTemporal); 
std::vector<size_t> regionQuery(signalData* signalDataArray, size_t pIndex, double epsSpatial, double epsTemporal, size_t dataPacketsInBuffer);
void expandCluster(signalData* signalDataArray, int16_t* signalGroupID, size_t pIndex, std::vector<size_t>& neighbors, int clusterId, double epsSpatial, double epsTemporal, int minPts, size_t dataPacketsInBuffer, std::unordered_map<int, clusterInfo>& clusterDetails); 
void ST_DBSCAN(signalData* signalDataArray, int16_t* signalGroupID, double epsSpatial, double epsTemporal, int minPts, size_t dataPacketsInBuffer);


#endif