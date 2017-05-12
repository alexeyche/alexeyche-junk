# export C_INCLUDE_PATH=/System/Library/Frameworks/Python.framework/Headers:/System/Library/Frameworks/Python.framework/Versions/2.7/Extras/lib/python/numpy/core/include
# g++ -O3 -fPIC -c plugin.cpp -std=c++14 -o net_step.o 
# ld -dylib -o net_step.so -lc net_step.o \
# 	 -L/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config -lpython2.7 \
# 	 -L/usr/lib -lstdc++

g++ -O3 -shared -fPIC ./net_step.cpp -std=c++14 -o ./net_step.so