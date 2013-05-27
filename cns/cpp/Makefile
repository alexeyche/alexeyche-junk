CC=g++
CFLAGS=-c -Wall -O1 -I/home/alexeyche/distr/credis-0.2.3
LDFLAGS=-larmadillo -L/home/alexeyche/distr/credis-0.2.3 -lcredis
SOURCES=main.cpp neurons.cpp simenv.cpp poisson.cpp connection.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=sim

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	/bin/rm -r -f $(OBJECTS) sim *.o *~


.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
