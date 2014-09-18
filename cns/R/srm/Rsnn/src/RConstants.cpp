#include "RConstants.h"

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

void RConstants::setValue(std::string section, std::string name, SEXP value) {
    if(TYPEOF(value) != STRSXP) {
        Rcpp::NumericVector v(value);
        std::ostringstream strs;
        for(size_t i=0; i<v.size(); i++) {
            if(i>= c->lc->size) {
                std::cout << "Too many values. Add more layers \n";
                break;
            }

            if(i>0) strs << " ";
            strs << v[i];
        }
        writeOption(section, name, strs.str());
    } else {
        Rcpp::CharacterVector v(value);
        assert(v.size() > 0);
        std::string fv(v[0]);
        if(fv.find(' ') != std::string::npos) {
            if(v.size() > 1) {
                std::cout << "Too messy value\n";
                return;
            }
            std::vector<std::string> x = split(fv, ' ');
            std::string acc_str("");
            for(size_t i=0; i<x.size(); i++) {
                if(i>= c->lc->size) {
                    std::cout << "Too many values. Add more layers \n";
                    break;
                }

                if(i>0) acc_str += " ";
                acc_str += x[i];
            }
            writeOption(section, name, acc_str);
        } else {
            std::string acc_str("");
            for(size_t i=0; i<v.size(); i++) {
                if(i>= c->lc->size) {
                    std::cout << "Too many values. Add more layers \n";
                    break;
                }

                if(i>0) acc_str += " ";
                acc_str += v[i];
            }
            writeOption(section, name, acc_str);
        }
    }
}

