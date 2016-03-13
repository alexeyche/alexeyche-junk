#!/usr/bin/env bash
packagename=Rdnn
ver=1.0
pushd $(dirname $(python -c 'import os,sys;print os.path.realpath(sys.argv[1])' $0))
set -x
pushd $packagename
Rscript -e 'Rcpp::compileAttributes()'
popd
DNN_INCLUDE=${DNN_INCLUDE:-$DNN_HOME/include}
DNN_LIB=${DNN_LIB:-$DNN_HOME/lib}
R CMD build $packagename
R CMD INSTALL --build ${packagename}_${ver}.tar.gz --configure-args="--with-dnn-include=$DNN_INCLUDE --with-dnn-lib=$DNN_LIB" --no-test-load
popd
if [ $(uname) == "Darwin" ]; then
    R_PKG_DIR=$(echo "cat(.libPaths()[1], '\n')" | R --slave  2>/dev/null | tr -d ' ')
    for l in $(ls $DNN_HOME/lib/*.dylib); do
        install_name_tool -change $(basename $l) $l $R_PKG_DIR/Rdnn/libs/Rdnn.so
    done    
    install_name_tool -change libdnn_protos.dylib $DNN_HOME/lib/libdnn_protos.dylib $DNN_HOME/lib/libdnn.dylib
fi    
