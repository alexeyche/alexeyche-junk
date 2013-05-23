CC=g++
CFLAGS=-c -Wall
LDFLAGS=-larmadillo
SOURCES=main.cpp neurons.cpp simenv.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=sim

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	/bin/rm -r -f $(OBJECTS) sim *.o *~



.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
