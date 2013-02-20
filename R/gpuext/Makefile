gpuext:
	nvcc -g -G -O1 -I/usr/share/R/include -I .. -Xcompiler "-Wall -Wextra -fpic" -c gpuext.cu gpuext.o
	nvcc -shared -lcublas gpuext.o -o gpuext.so
clean:
	rm -rf *.o *.so
