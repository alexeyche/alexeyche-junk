#ifndef COMMON_H
#define COMMON_H

#define ERR(err) \
    std::stringstream ss;\
    ss << err;  \
    Rcpp::stop(ss.str());\




#endif
