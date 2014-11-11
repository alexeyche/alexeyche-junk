#ifndef CORE_H
#define CORE_H

#include <armadillo>
#include <stdio.h>
#include <string>

using namespace arma;


#include <sim/util/log.hpp>
#include <sim/util/cli.hpp>
#include <sim/util/timers.hpp>

using namespace sim;
namespace srm {
    struct SrmException : public std::exception
    {
       std::string s;
       SrmException(std::string ss) : s(ss) {}
       ~SrmException() throw () {} // Updated
       const char* what() const throw() { return s.c_str(); }
    };
};    

#endif
