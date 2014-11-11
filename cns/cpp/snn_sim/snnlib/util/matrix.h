#pragma once

#include <snnlib/core.h>

using namespace std;


template <typename T>
class Matrix {
public:    
    
    Matrix(size_t nr, size_t nc) : nrow(nr), ncol(nc) {
        allocate(nrow, ncol);
    }
    ~Matrix() {
        if((nrow != 0)&&(ncol != 0)) delete[] vals;
    }
    Matrix() : nrow(0), ncol(0) {
    }
    
    T getElement(size_t i, size_t j) {
        assert( (nrow != 0) && (ncol != 0) );
        return vals[j*nrow + i];
    }
    void setElement(size_t i, size_t j, T val) {
        assert( (nrow != 0) && (ncol != 0) );
        vals[j*nrow + i] = val;
    }
    void allocate(size_t nr, size_t nc) {
        if((nrow != 0)&&(ncol != 0)) delete[] vals;
        nrow = nr;
        ncol = nc;
        vals = new T[nrow*ncol];
    }
    void fill(T val) {
        for(size_t i=0; i<nrow; i++) {
            for(size_t j=0; j<ncol; j++) {
                setElement(val);
            }
        }
    }
    void fill_from_slice(const T *val_slice, size_t slice_size) {
        if(slice_size != nrow*ncol) {
            stringstream ss; ss << "Can't fill matrix with size" << nrow << ":" << ncol << "  with slize with size " << slice_size << "\n";
            throw exception(ss.str());
        }
    }

private:        
    T *vals;
    size_t ncol;
    size_t nrow;
};

