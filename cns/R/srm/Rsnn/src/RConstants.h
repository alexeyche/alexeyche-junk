#ifndef RCONSTANTS_H
#define RCONSTANTS_H

#include <Rcpp.h>

extern "C" {
    #include <constants.h>
}

class RConstants {
public:
    RConstants(std::string _const_filename) : const_filename(_const_filename) { 
        c = createConstants(const_filename.c_str());
    }
    void print() {
        printConstants(c);
    }
    ~RConstants() {
        deleteConstants(c);
    }
    std::string const_filename;
    Constants *c;
};

#endif
