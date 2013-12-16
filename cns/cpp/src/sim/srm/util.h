#ifndef UTIL_H
#define UTIL_H

#include <sim/core.h>
#include <sim/util/confParser.h>

namespace srm {
    bool fileExist( const std::string& name );
    std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
    std::vector<std::string> split(const std::string &s, char delim);
    bool dirExist(const std::string& name);
    std::vector<std::string> listDir (const std::string &dir);
    std::string formModelName(const TConfig &c, const double &learning_rate);

};

#endif
