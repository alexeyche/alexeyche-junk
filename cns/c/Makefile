CFLAGS = -std=c99 -Wmain -Wall -DNDEBUG -lactor
OBJS = layer.o

all: srm_sim

srm_sim: srm_sim.o $(OBJS)
	gcc srm_sim.o -o srm_sim $(CFLAGS) $(OBJS) 

clean:
	/bin/rm -r -f $(OBJS) srm_sim *.o *~
.c.o:
	gcc -c $(CFLAGS) $<
