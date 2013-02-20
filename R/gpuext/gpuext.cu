#include <R.h>
#include <Rdefines.h>
#include <cudamat/cudamat.cu>

char *make_info_str(cudamat *mat) {
    char *info = (char*)malloc(sizeof(char)*150);
    snprintf(info,150,"host: %p dev: %p on_device: %i on_host: %i size: %ix%i trans: %i", mat->data_host,mat->data_device,mat->on_device, mat->on_host, mat->size[0], mat->size[1],mat->is_trans);
    return info;
}


extern "C" {
    SEXP cublas_init_R() {
        cublasInit();
        SEXP result;
        char *result_p = (char*)malloc(sizeof(char)*100);
        if(check_cublas_error()) {
            sprintf(result_p, "Error: %c", CUBLAS_ERROR);
            result = mkString(result_p);
        } else {
             result_p = "CUBLAS initialised";
             result = mkString(result_p);
       }
       return result;
    }

    SEXP cublas_shutdown_R() {
        cublasShutdown();
        SEXP result;
        char *result_p = (char*)malloc(sizeof(char)*100);
        if(check_cublas_error()) {
            sprintf(result_p, "Error: %c", CUBLAS_ERROR);
            result = mkString(result_p);
        } else {
             result_p = "CUBLAS shutdown";
             result = mkString(result_p);
       }
       return result;
    }

    static void _finalizer(SEXP ext)
    {
           struct cudamat *ptr = (struct cudamat*) R_ExternalPtrAddr(ext);
           Free(ptr);
           //!!! NEED FREE CUDA ARRAYS
    }
    
    SEXP init_from_array_R(SEXP matrix) {
        SEXP mat_ext, info, list, list_names;
        float *mat_dev;
        int nrow, ncol;
        // create cudamat object from struct
        struct cudamat *mat = (cudamat*)malloc(sizeof(cudamat));
        // create float array from matrix
        if(isMatrix(matrix) && isReal(matrix)) {
            double *mat_d = REAL(matrix);
            ncol = INTEGER(GET_DIM(matrix))[1];
            nrow = INTEGER(GET_DIM(matrix))[0];
            mat_dev = (float*)malloc(sizeof(ncol*nrow));
            for(int row=0; row < nrow; row++) {
                for(int col=0; col < ncol; col++) {
                    mat_dev[col*ncol+row] = (float) mat_d[col*ncol+row];  // float cast
                }
            }
        } else {
            return mkString("Need matrix as input");
        }
        init_from_array(mat,mat_dev,nrow,ncol);
        // make ref
        mat_ext = PROTECT(R_MakeExternalPtr(mat, R_NilValue, R_NilValue));
        R_RegisterCFinalizerEx(mat_ext, _finalizer, Rboolean(TRUE));
        UNPROTECT(1);
        //create info part
        char *info_p = make_info_str(mat);
        info = PROTECT(mkString((char*)info_p));
        // generate list names
        char *names[2] = {"ref","info"};
        PROTECT(list_names = allocVector(STRSXP, 2));
        for(char i = 0; i < 2; i++)
            SET_STRING_ELT(list_names, i,  mkChar(names[i]));
        // set array
        PROTECT(list = allocVector(VECSXP, 2)); // Creating a list with 2 vector elements
        
        SET_VECTOR_ELT(list, 0, mat_ext);
        SET_VECTOR_ELT(list, 1, info);
        setAttrib(list, R_NamesSymbol, list_names); //and attaching the vector names
        UNPROTECT(3);
        // return list with ref
        return list;
    }
}
