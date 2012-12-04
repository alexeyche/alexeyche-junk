CFLAGS = -Wall -DNDEBUG -g
#CFLAGS = -g -Wall
OBJS = maxent.o lbfgs.o owlqn.o sgd.o

all: bicycle postagging mymaxent mymaxent_pred

bicycle: bicycle.o $(OBJS)
	g++ -o bicycle $(CFLAGS) $(OBJS) bicycle.o
postagging: postagging.o $(OBJS)
	g++ -o postagging $(CFLAGS) $(OBJS) postagging.o
mymaxent: mymaxent.o $(OBJS)
	g++ -o mymaxent $(CFLAGS) $(OBJS) mymaxent.o
mymaxent_pred: mymaxent_pred.o $(OBJS)
	g++ -o mymaxent_pred $(CFLAGS) $(OBJS) mymaxent_pred.o

clean:
	/bin/rm -r -f $(OBJS) bicycle postagging mymaxent mymaxent_pred *.o *~ 
.cpp.o:
	g++ -c $(CFLAGS) $<
