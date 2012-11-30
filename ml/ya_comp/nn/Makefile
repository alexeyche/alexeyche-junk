CFLAGS = -O2 -Wall -DNDEBUG -g
OBJS = backprop.o 

all: nn_bp

nn_bp: neuralnet.o $(OBJS)
	g++ -o nn_bp $(CFLAGS) $(OBJS) neuralnet.o

clean:
	/bin/rm -r -f $(OBJS) nn_bp *.o *~
.cpp.o:
	g++ -c $(CFLAGS) $<
