CFLAGS = -Wall -DNDEBUG -g #-O2 -Wall -DNDEBUG -g
OBJS = 

all: knn

knn: knn.o $(OBJS)
	g++ -o knn $(CFLAGS) $(OBJS) knn.o

clean:
	/bin/rm -r -f $(OBJS) knn *.o *~
.cpp.o:
	g++ -c $(CFLAGS) $<
