#!/usr/bin/env bash
packagename=Rsnn
ver=1.0

set -x
pushd $packagename
Rscript -e 'Rcpp::compileAttributes()'
popd
SNN_INC="/usr/local/include/snnlib"
SNN_LIB="/usr/local/lib"
R CMD build $packagename
R CMD INSTALL --build ${packagename}_${ver}.tar.gz --configure-args="--with-snn-include=$SNN_INC --with-snn-lib=$SNN_LIB"
