#pragma once


// #include <snnlib/core.h>
// #include <snnlib/util/json/json_box.h>
// #include <snnlib/serialize/serialize.h>
// #include <snnlib/protos/matrix.pb.h>



// template <typename T>
// class Matrix {
// public:

//     Matrix(size_t nr, size_t nc) : Matrix() {
//         allocate(nr, nc);
//     }
//     Matrix(size_t nr, size_t nc, T val) : Matrix(nr, nc) {
//         fill(val);
//     }
//     ~Matrix() {
//         if((nrow != 0)&&(ncol != 0)) delete[] vals;
//     }
//     Matrix() : nrow(0), ncol(0) {
//     }

//     T getElement(size_t i, size_t j) const {
//         assert( (nrow != 0) && (ncol != 0) );
//         return vals[j*nrow + i];
//     }
//     void setElement(size_t i, size_t j, T val) {
//         assert( (nrow != 0) && (ncol != 0) );
//         vals[j*nrow + i] = val;
//     }
//     void allocate(size_t nr, size_t nc) {
//         if((nrow != 0)&&(ncol != 0)) delete[] vals;
//         nrow = nr;
//         ncol = nc;
//         vals = new T[nrow*ncol];
//     }
//     void fill(T val) {
//         for(size_t i=0; i<nrow; i++) {
//             for(size_t j=0; j<ncol; j++) {
//                 setElement(i, j, val);
//             }
//         }
//     }
//     void print(std::ostream& str) const {
//         for(size_t i=0; i<nrow; i++) {
//             str << "|";
//             for(size_t j=0; j<nrow; j++) {
//                 str << getElement(i, j);
//                 if( j < (nrow-1) ) {
//                     str << " | ";
//                 }
//             }
//             str << "|\n";
//         }
//     }
//     void printR(const vector<string> &row_labels, const vector<string> &col_labels) {
//         assert( (row_labels.size() == nrow ) && (col_labels.size() == ncol) );
//         for(auto it=col_labels.begin(); it != col_labels.end(); ++it) {
//             cout << *it;
//             if( (it+1) != col_labels.end() ) cout << ",";
//         }
//         cout << "\n";
//         for(size_t i=0; i<nrow; i++) {
//             cout << row_labels[i] << ",";
//             for(size_t j=0; j<ncol; j++) {
//                 cout << getElement(i, j);
//                 if( (j+1) != ncol ) cout << ",";
//             }
//             cout << "\n";
//         }
//     }

// protected:
//     T *vals;
//     size_t ncol;
//     size_t nrow;
// };

// class DoubleMatrix: public Matrix<double>, public Serializable<Protos::DoubleMatrix> {
// public:
//     DoubleMatrix(size_t nr, size_t nc) : Matrix<double>(nr, nc), Serializable(EDoubleMatrix) {}
//     DoubleMatrix(size_t nr, size_t nc, double val) : Matrix<double>(nr, nc, val), Serializable(EDoubleMatrix) {}
//     DoubleMatrix() : Serializable(EDoubleMatrix) {}
//     ProtoPack serialize() {
//         Protos::DoubleMatrix *m = getNewMessage();
//         m->set_ncol(ncol);
//         m->set_nrow(nrow);
//         for(size_t i=0; i<(ncol*nrow); i++) {
//             m->add_vals(vals[i]);
//         }
//         return ProtoPack({m});
//     }

//     void print(std::ostream& str) const {
//         Matrix<double>::print(str);
//     }


// };

#include <dnn/io/serialize.h>
#include <dnn/protos/generated.pb.h>

namespace dnn {

/*@GENERATE_PROTO@*/
class DoubleMatrix : public Serializable<Protos::DoubleMatrix> {
public:
    void serial_process() {
    	begin() << "ncol_v: " << ncol_v << ", " 
    			<< "nrow_v: " << nrow_v << ", " 
    			<< "vals: " << vals << Self::end;

    }
    double& operator () (size_t i, size_t j) {
        assert( (nrow_v != 0) && (ncol_v != 0) );
        if( ! ( (i<nrow_v) && (j<ncol_v) ) ) {
            throw dnnException() << "assert: " << i << "<" << nrow_v << " && " << j << "<" << ncol_v << "\n";
        }
        return vals[j*nrow_v + i];
    }
    
    double operator () (size_t i, size_t j) const {
        assert( (nrow_v != 0) && (ncol_v != 0) );
        if( ! ( (i<nrow_v) && (j<ncol_v) ) ) {
            throw dnnException() << "assert: " << i << "<" << nrow_v << " && " << j << "<" << ncol_v << "\n";
        }
        return vals[j*nrow_v + i];
    }
    DoubleMatrix(size_t _nrow, size_t _ncol) {
        allocate(_nrow, _ncol);
    }

    DoubleMatrix() : nrow_v(0), ncol_v(0) {
    }
    DoubleMatrix(const vector<double> &v) {
    	allocate(v.size(), 1);
    	for(size_t i=0; i<v.size(); ++i) {
    		setElement(i, 0, v[i]);
    	}
    }

    double getElement(size_t i, size_t j) const {
        assert( (nrow_v != 0) && (ncol_v != 0) );
        if( ! ( (i<nrow_v) && (j<ncol_v) ) ) {
            throw dnnException() << "assert: " << i << "<" << nrow_v << " && " << j << "<" << ncol_v << "\n";
        }
        return vals[j*nrow_v + i];
    }
    void setElement(size_t i, size_t j, double val) {
        assert( (nrow_v != 0) && (ncol_v != 0) );
        if( ! ( (i<nrow_v) && (j<ncol_v) ) ) {
            throw dnnException() << "assert: " << i << "<" << nrow_v << " && " << j << "<" << ncol_v << "\n";
        }
        vals[j*nrow_v + i] = val;
    }
    void allocate(size_t nr, size_t nc) {
        if((nrow_v != 0)&&(ncol_v != 0)) vals.clear();
        nrow_v = nr;
        ncol_v = nc;
        vals.resize(nrow_v*ncol_v);
	}
    void fill(double val) {
	    for(size_t i=0; i<nrow_v; i++) {
    	    for(size_t j=0; j<ncol_v; j++) {
        	    setElement(i, j, val);
        	}
    	}
	}
    void norm() {
        for(size_t i=0; i<nrow(); ++i) {
            double acc = 0.0;
            for(size_t j=0; j<ncol(); ++j) {
                acc += getElement(i, j) * getElement(i, j);                 
            }
            double n = sqrt(acc);
            for(size_t j=0; j<ncol(); ++j) {
                setElement(i, j, getElement(i, j)/n);
            }
        }
    }
	inline const size_t& ncol() const { return ncol_v; } 	
	inline const size_t& nrow() const { return nrow_v; } 	
private:

	size_t ncol_v;
	size_t nrow_v;
	vector<double> vals;
};



}
