#!/usr/bin/env bash

set -x 

nvcc -o ./main -L/home/alexeyche/distr/opencv-build/lib/ -lopencv_gpu -lopencv_highgui -lopencv_core -lopencv_imgproc ./main.cu
