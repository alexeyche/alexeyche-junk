#!/usr/bin/env bash
packagename=Rdnn
ver=1.0
pushd $(dirname $(readlink -f $0))
set -x
pushd $packagename
Rscript -e 'Rcpp::compileAttributes()'
popd
DNN_INC="/usr/local/include/dnn"
DNN_LIB="/usr/local/lib"
R CMD build $packagename
R CMD INSTALL --build ${packagename}_${ver}.tar.gz --configure-args="--with-snn-include=$DNN_INC --with-snn-lib=$DNN_LIB"
popd
