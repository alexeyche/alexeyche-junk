#ifndef RCONSTANTS_H
#define RCONSTANTS_H

#include <dnn/base/constants.h>

#undef PI
#define STRICT_R_HEADERS
#include <Rcpp.h>


//using namespace std;

class RConstants : public dnn::Constants {
public:
    RConstants(std::string _const_json) : Constants(_const_json, dnn::OptMods(), Constants::FromString) { 
    	sim_conf.files.clear();
    }
    void print() {
        cout << *this;
    }
};

#endif
