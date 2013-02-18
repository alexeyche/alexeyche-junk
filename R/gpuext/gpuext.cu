#include <R.h>
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
}
