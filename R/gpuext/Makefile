gpuext:
	nvcc -g -G -O2 -I/usr/share/R/include -Xcompiler "-Wall -Wextra -fpic" -c gpuext.cu gpuext.o
	nvcc -shared -lcublas gpuext.o -o gpuext.so
clean:
	rm -rf *.o *.so
