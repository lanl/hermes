#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include "structures.h"
#include <string>

bool readConfigFile(const std::string &filename, configParameters &params);
void printParameters(const configParameters &params) ;

#endif // CONFIG_READER_H
