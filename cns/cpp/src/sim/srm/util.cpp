

#include "util.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

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
    bool dirExist(const std::string &name) {
        struct stat s;
        int err = stat(name.c_str(), &s);
        if(-1 == err) {
            if(ENOENT == errno) {
                return false;
            } else {
                return false;
            }
        } else {
            if(S_ISDIR(s.st_mode)) {
                return true;
            } else {
                return false;
            }
        } 
    }
    
    std::vector<std::string> listDir(const std::string &dir) {
        std::vector<std::string> files;
        DIR *dp;
        struct dirent *dirp;
        if((dp  = opendir(dir.c_str())) == NULL) {
            cout << "Error(" << errno << ") opening " << dir << endl;
            throw SrmException("Error");
        }

        while ((dirp = readdir(dp)) != NULL) {
            files.push_back(std::string(dirp->d_name));
        }
        closedir(dp);
        return files;
    }
    std::string formModelName(const TConfig &c, const double &learning_rate) {
        char lr[10];
        sprintf(lr, "%.2f", learning_rate);
        return  "weights" +
                std::to_string(c.nneurons_in)+"x"+ std::to_string(c.nneurons_out)+
                "_lr"+ std::string(lr)+
                ".csv";
    }
};

