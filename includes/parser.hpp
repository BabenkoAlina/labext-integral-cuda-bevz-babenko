#ifndef PARSER_INCLUDE
#define PARSERL_INCLUDE

#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <list>
#include <regex>
#include <map>
#include <sstream>


std::map<std::string, double> getParametersFromConfig(const std::string& filename) {
    std::ifstream file(filename);
    std::map<std::string, double> configMap;
    std::string line;

    if (!file.is_open()) {
        throw std::runtime_error("Error opening configuration file");
    }

    while (std::getline(file, line)) {
        size_t findLine = line.find('=');

        if (findLine != std::string::npos) {
            std::string key = line.substr(0, findLine);
            std::string valueStr = line.substr(findLine + 1);

            std::istringstream valueStream(valueStr);
            double value;
            if (!(valueStream >> value)) {
                throw std::invalid_argument("Error converting value to double");
            }
            configMap.insert(std::make_pair(key, value));
        }
    }
    return configMap;
}

#endif // PARSER_INCLUDE