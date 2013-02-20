#include <R.h>
#include <Rdefines.h>
#include <cudamat/cudamat.cu>

char *make_info_str(cudamat *mat) {
    char *info = (char*)malloc(sizeof(char)*150);
    snprintf(info,150,"host: %p dev: %p on_device: %i on_host: %i size: %ix%i trans: %i", mat->data_host,mat->data_device,mat->on_device, mat->on_host, mat->size[0], mat->size[1],mat->is_trans);
    return info;
}



void generate_exception(int err_code) {
    if  ( err_code == -1) {
       Rprintf("Incompatible matrix dimensions.\n");
    } else if  ( err_code == -2) {
       Rprintf("CUBLAS error.\n");
    } else if  ( err_code == -3) {
       Rprintf("CUDA error: ");
       char *cuda_error = get_last_cuda_error(); 
       Rprintf(cuda_error); 
    } else if  ( err_code == -4) {
       Rprintf("Operation not supported on views.\n");
    } else if  ( err_code == -5) {
       Rprintf("Operation not supported on transposed matrices.\n");
    } else if  ( err_code == -6) {
       Rprintf("\n");
    } else if  ( err_code == -7) {
       Rprintf("Incompatible transposedness.\n");
    } else if  ( err_code == -8) {
       Rprintf("Matrix is not in device memory.\n");
    } else if  ( err_code == -9) {
       Rprintf("Operation not supported.\n");
    }
}


const char *mat_ext_names[2] = {"ref","info"};


SEXP generate_list(cudamat *mat) {
    SEXP mat_ext, list, list_names;
    mat_ext = PROTECT(R_MakeExternalPtr(mat, R_NilValue, R_NilValue));
    R_RegisterCFinalizerEx(mat_ext, _finalizer, Rboolean(TRUE));
    UNPROTECT(1);
    
    char *info_p = make_info_str(mat);
    info = PROTECT(mkString((char*)info_p));
    PROTECT(list_names = allocVector(STRSXP, 2));
    for(char i = 0; i < 2; i++)
        SET_STRING_ELT(list_names, i,  mkChar(mat_ext_names[i]));
    // set array
    PROTECT(list = allocVector(VECSXP, 2)); // Creating a list with 2 vector elements
    
    SET_VECTOR_ELT(list, 0, mat);
    SET_VECTOR_ELT(list, 1, info);
    setAttrib(list, R_NamesSymbol, list_names); //and attaching the vector names
    UNPROTECT(3);
    return list;
}

cudamat* process_list(SEXP list) {
    SEXP elmt;
    if((length(list) == 2) && (strcmp(CHAR(STRING_ELT(names, 0)), mat_ext_names[0]) == 0) && (strcmp(CHAR(STRING_ELT(names, 1)), mat_ext_names[1]) == 0)) {
        elmt = getListElement(mat_ext,mat_ext_names[0]);
        return (struct cudamat*) R_ExternalPtrAddr(elmt);
    } else {
        Rprintf("Input not look like cuda matrix\n");        
        return 0;
    }
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
           int error = free_device_memory(ptr);
           if(error>0) {
                generate_exception(error);
           }
           Free(ptr);
    }
    
    SEXP init_from_array_R(SEXP matrix) {
        SEXP list;
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
        // generate list with inf
        list = generate_list(mat);
        // return list with ref
        return list;
    }
    
    SEXP copy_to_device_R(SEXP mat_ext) {
        SEXP list, elmt = R_NilValue, names = getAttrib(list, R_NamesSymbol);
        
        struct cudamat* m = process_list(list);
        if (m) {
            int error = copy_to_device(ptr); 
        
            if(error > 0) {
                 generate_exception(error);           
                 return R_NilValue;
            }
            
            list = generate_list(ptr);                     
            return list;
        }
        return R_NilValue;
    }
}
