#pragma once

#include <sstream>

using std::stringstream;

namespace NDnn {

    #define DEFINE_ERROR(type) \
        struct type : public std::exception \
        { \
            type() {} \
            type(type &exc) { \
                ss << exc.ss.str(); \
            } \
            \
            template <typename T> \
            type& operator << (const T& s) { \
                ss << s; \
                return *this; \
            } \
            \
            const char * what () const throw () { \
                return ss.str().c_str(); \
            } \
            stringstream ss; \
        };

    DEFINE_ERROR(TDnnException);
    DEFINE_ERROR(TDnnInterrupt);
    DEFINE_ERROR(TDnnNotImplemented);
    DEFINE_ERROR(TDnnFileNotFound);
    DEFINE_ERROR(TDnnElementNotFound);
    DEFINE_ERROR(TDnnLogicError);
    DEFINE_ERROR(TDnnAlgebraError);
    DEFINE_ERROR(TDnnNotAvailable);

    #define ENSURE(cond, str) \
        if(!(cond)) { \
            throw TDnnException() << str; \
        }\

    #define ENSURE_ERR(cond, exc) \
        if(!(cond)) { \
            throw exc; \
        }\


} // namespace NDnn
