#ifndef ACTOR_SIM_H
#define ACTOR_SIM_H

typedef enum { false, true } bool;

typedef struct {
    double t;
} Tick;

typedef struct {
    double t;
    actor_id act_id;
    int id;
} ActorSim;

typedef struct {
    ActorSim *arr;
    size_t n;
    double t;
} DispSim;



enum {
	CALC_DONE_MSG = 100,
    TICK_MSG = 101,
    EXIT_MSG = 102
};


#endif
