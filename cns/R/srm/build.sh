#!/usr/bin/env bash
packagename=snn
ver=1.0

set -x
pushd $packagename
Rscript -e 'Rcpp::compileAttributes()'
popd
R CMD build $packagename
R CMD INSTALL --build ${packagename}_${ver}.tar.gz
