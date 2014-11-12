#pragma once

#include <snnlib/core.h>
#include <snnlib/util/json/json_box.h>

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
    
    T getElement(size_t i, size_t j) const {
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
    void fill_from_json(JsonBox::Array a) {
        if(a.size() == 0) {
            cerr <<  "Can't fill matrix with null array json\n"; 
            terminate();
        }
        if(a.size() != nrow) {
            cerr << "Can't fill matrix: row sizes not match " << a.size() << " != " << nrow << "\n";
            terminate();
        }
        for(size_t i=0; i < a.size(); i++) {
            auto a_row = a[i].getArray();
            if(a_row.size() != ncol) {
                cerr << "Can't fill matrix: col sizes not match " << a_row.size() << " != " << ncol << "\n";
                terminate();
            }
            for(size_t j=0; j < a_row.size(); j++) {
                setElement(i, j, a_row[j].getDouble());
            }
        }
    }
    friend std::ostream& operator<<(std::ostream& str, Matrix<T> const& data) {
        for(size_t i=0; i<data.nrow; i++) {
            for(size_t j=0; j<data.nrow; j++) {
                str << data.getElement(i, j);
                if( j < (data.nrow-1) ) {
                    str << "|";
                }
            }
            str << "\n";
        }
    }
private:        
    T *vals;
    size_t ncol;
    size_t nrow;
};

