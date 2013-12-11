

#include "util.h"

namespace srm {
    
    bool fileExist( const std::string& name )
    {
         std::ifstream f(name.c_str());  // New enough C++ library will accept just name
         return f.is_open();
    }

    std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }

    std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, elems);
        return elems;
    }

};

