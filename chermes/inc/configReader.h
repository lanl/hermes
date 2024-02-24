#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <string>

struct configParameters {
    std::string tpxFileName;
    bool writeRawSignals = false;
    int maxBuffersToRead = 0;
    bool sortSignals = false;
    bool verbose = false;
    bool fillHistgrams = false;
    bool clusterPixels = false;
    int epsSpatial = 0;
    double epsTemporal = 0;
    int minPts = 0;
};

bool readConfigFile(const std::string &filename, configParameters &params);

#endif // CONFIG_READER_H
