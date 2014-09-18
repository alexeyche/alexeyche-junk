#ifndef RCONSTANTS_H
#define RCONSTANTS_H

#include <Rcpp.h>

extern "C" {
    #include <constants.h>
}


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

class RConstants {
public:
    RConstants(std::string _const_filename) : const_filename(_const_filename) { 
        c = createConstants(const_filename.c_str());
    }
    void print() {
        printConstants(c);
    }

    void setValue(std::string section, std::string name, SEXP value);

    void writeOption(std::string section, std::string name, std::string value) {
        int ret_code = file_handler((void*)c, section.c_str(), name.c_str(), value.c_str());
        if(ret_code == 0) {
            std::cout << "Error while searching for " << section << " " << name << " " << value << " in Constants\n"; 
        }

    }
    ~RConstants() {
        deleteConstants(c);
    }
    std::string const_filename;
    Constants *c;
};

#endif
