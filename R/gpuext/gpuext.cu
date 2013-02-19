#include <R.h>
#include <Rdefines.h>
#include "cudamat.cu"




extern "C" {
    void cublas_init_R(char **result) {
        cublasInit();
        if(check_cublas_error()) {
            sprintf(*result, "Error: %c", CUBLAS_ERROR);
        } else {
            *result = "CUBLAS initialised";
        }
    }

    void cublas_shutdown_R(char **result) {
        cublasShutdown();
        if(check_cublas_error()) {
            sprintf(*result, "Error: %c", CUBLAS_ERROR);
        } else {
            *result = "CUBLAS shutdown";
        }
    }
    
    SEXP init_from_array_R(SEXP matrix) {
        // create cudamat object from struct
        struct cudamat *mat = Calloc(1, struct cudamat);
        // create float array from matrix
        // make ref
        SEXP mat_ext = PROTECT(R_MakeExternalPtr(mat, R_NilValue, R_NilValue));
        R_RegisterCFinalizerEx(mat_ext, _finalizer, TRUE);
        UNPROTECT(1);
        // generate list
        char *names[2] = {"ref","info"};
        
        PROTECT(list_names = allocVector(STRSXP, 1));
        
        for(i = 0; i < 1; i++)
            SET_STRING_ELT(list_names, i,  mkChar(names[i]));
        
        PROTECT(list = allocVector(VECSXP, 2)); // Creating a list with 2 vector elements
        
        SET_VECTOR_ELT(list, 0, mat_ext);
        SET_VECTOR_ELT(list, 0, info);
        setAttrib(list, R_NamesSymbol, list_names); //and attaching the vector names
        UNPROTECT(3);
        // return list with ref
        return list;
    }
}
