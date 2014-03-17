#!/usr/bin/env bash

mkdir ~/distr
cd ~/distr


sudo apt-get install g++

sudo apt-get install gfortran

cp /mnt/yandex.disk/distr/xianyi-OpenBLAS-v0.2.8-0-g835293c.tar.gz .
openBlasTar=$(find ./ -type f -name "xianyi-OpenBLAS-*")
tar -xf $openBlasTar
openBlasDir=$(find ./ -type d -name "xianyi-OpenBLAS-*")
pushd $openBlasDir
./configure
make -j8
make install
popd

sudo apt-get install cmake

cp /mnt/yandex.disk/distr/armadillo-4.100.2.tar.gz .
tar -xf armadillo-4.100.2.tar.gz
pushd armadillo-4.100.2
cmake .
make -j8
sudo make install
popd



sudo apt-get install libreadline6 libreadline6-dev xorg-dev libpng-dev libjpeg62 texlive-latex-base
wget http://cran.rstudio.com/src/base/R-3/R-3.0.2.tar.gz
tar -xf ./R-3.0.2.tar.gz
pushd R-3.0.2
./configure --enable-R-shlib --enable-BLAS-shlib

make -j8
make install
popd

mkdir -p ~/prog/sim/ts
pushd ~/prog/ts
wget http://www.cs.ucr.edu/~eamonn/time_series_data/dataset.zip
wget http://www.cs.ucr.edu/~eamonn/time_series_data/dataset2.zip
popd

RSCR=$(cat << END_OF_R_INSTALL
options(repos=structure(c(CRAN="http://cran.r-project.org/")))
install.packages('lattice')
install.packages('Rcpp')
install.packages('RcppArmadillo')
install.packages('zoo')
install.packages('abind')
install.packages('reshape2')
install.packages('ggplot2')
install.packages('neuralnet')
END_OF_R_INSTALL
)

echo "$RSCR" > R_prelim.R
R -q --no-save --file=R_prelim.R


sudo apt-get install git
pushd ~/prog
git clone https://github.com/alexeyche/alexeyche-junk
popd
