#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <string>

struct configParameters {
    std::string tpxFileName;
    int epsSpatial = 0;
    double epsTemporal = 0;
    int minPts = 0;
    bool sortSignals = false;
    bool writeRawSignals = false;
    bool verbose = false;
    bool fillHistgrams = false;
};

bool readConfigFile(const std::string &filename, configParameters &params);

#endif // CONFIG_READER_H
