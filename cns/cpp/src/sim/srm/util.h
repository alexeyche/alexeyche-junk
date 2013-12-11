#ifndef UTIL_H
#define UTIL_H

#include <sim/core.h>

namespace srm {
    bool fileExist( const std::string& name );
    std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
    std::vector<std::string> split(const std::string &s, char delim);
};

#endif
