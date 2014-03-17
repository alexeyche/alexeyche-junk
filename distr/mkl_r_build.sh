MKL_LIB_PATH=/opt/intel/composer_xe_2013_sp1.0.080/mkl/lib/intel64
MKLC_LIB_PATH=/opt/intel/composer_xe_2013_sp1.0.080/compiler/lib/intel64
MKL="   -L${MKL_LIB_PATH} -L${MKLC_LIB_PATH}            \
        -Wl,--start-group                               \
                ${MKL_LIB_PATH}/libmkl_gf_lp64.a        \
                ${MKL_LIB_PATH}/libmkl_gnu_thread.a     \
                ${MKL_LIB_PATH}/libmkl_core.a           \
        -Wl,--end-group                                 \
        -liomp5 -lpthread -lgomp -lm -ldl"
./configure --with-lapack="$MKL" --with-blas="$MKL" --enable-R-shlib
