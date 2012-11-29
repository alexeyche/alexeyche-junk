CFLAGS = -O2 -Wall -DNDEBUG -g
#CFLAGS = -g -Wall
OBJS = maxent.o lbfgs.o owlqn.o sgd.o

all: bicycle postagging mymaxent

bicycle: bicycle.o $(OBJS)
	g++ -o bicycle $(CFLAGS) $(OBJS) bicycle.o
postagging: postagging.o $(OBJS)
	g++ -o postagging $(CFLAGS) $(OBJS) postagging.o
mymaxent: mymaxent.o $(OBJS)
	g++ -o mymaxent $(CFLAGS) $(OBJS) mymaxent.o

clean:
	/bin/rm -r -f $(OBJS) bicycle postagging mymaxent *.o *~ model
.cpp.o:
	g++ -c $(CFLAGS) $<
