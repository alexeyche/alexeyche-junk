#!/usr/bin/env bash
packagename=Rdnn
ver=1.0
pushd $(dirname $(readlink -f $0))
set -x
pushd $packagename
Rscript -e 'Rcpp::compileAttributes()'
popd
DNN_INCLUDE=~/dnn/include
DNN_LIB=~/dnn/lib
R CMD build $packagename
R CMD INSTALL --build ${packagename}_${ver}.tar.gz --configure-args="--with-dnn-include=$DNN_INCLUDE --with-dnn-lib=$DNN_LIB"
popd
