
#include <libactor/actor.h>
#include <math.h>
#include <util/util.h>
#include <util/util_vector.h>

#include "actor_sim.h"

#define NUM_ACTORS 4
#define TMAX 1000

double calc_func(double i) {
    double p = fmod(i, 10);
    double ans1 = pow(55.0, p);
    double ans2 = pow(51.0, p);
    double ans3 = pow(56.0, p);
    double ans4 = pow(50.0, p);
    double a= (ans1/ans2)*ans3*i + ans4*i;
    return(a);
}

#define RATE 0.1
#define QUANT 2
#define DT 1

void* run_sim(void *args) {
    ActorSim *act = (ActorSim*)args;
	actor_msg_t *msg;
    bool exit = false;
    Tick *tk;
	while(!exit) {
		msg = actor_receive();
        if(msg->type == TICK_MSG) {
    		tk = (Tick*)msg->data;
            for(size_t t=0; t<QUANT; t+=DT) {
                if( RATE * DT > getUnif() ) {
                    double t_spike = tk->t+t;
//                    printf("We have a spike at %f in %d\n", tk->t+t,act->id);
                    actor_send_msg(msg->sender, SPIKE_MSG, (void*)&t_spike, sizeof(double));
                }
            }
            actor_send_msg(msg->sender, CALC_DONE_MSG, NULL, 0);
        } else
        if(msg->type == EXIT_MSG) {
            printf("quiting\n");
            exit=true;
        }
        arelease(msg);
    }
    return(0);
}

void* disp_run(void *args) {
    size_t n=NUM_ACTORS;
	actor_msg_t *msg;
    ActorSim *actors = (ActorSim*) malloc(n*sizeof(ActorSim));
    for(size_t ai=0; ai<n; ai++) {
        actors[ai].id = ai;
        actors[ai].act_id = spawn_actor(run_sim, (void*)&actors[ai]);
    }


    double t = 0;
    doubleVector *v = TEMPLATE(createVector,double)();
    while(t<TMAX) {
        Tick tk;
        tk.t = t;
//        actor_broadcast_msg(TICK_MSG, (void*)&tk, sizeof(Tick));
        for(size_t ai=0; ai<n; ai++) {
            actor_send_msg(actors[ai].act_id, TICK_MSG, (void*)&tk, sizeof(Tick));
        }
  		int nanswers=0;
        while(nanswers<n) {
            msg = actor_receive();
            if(msg->type == CALC_DONE_MSG) {
                nanswers+=1;
            } else 
            if(msg->type == SPIKE_MSG) {
                TEMPLATE(insertVector,double)(v, *(double*)msg->data);
            }
            arelease(msg);
        }
        t+=QUANT;
    }
    for(size_t ai=0; ai<n; ai++) {
        actor_send_msg(actors[ai].act_id, EXIT_MSG, NULL, 0);
    }
    printf("We had spikes:\n");
    for(size_t sp_i=0; sp_i<v->size; sp_i++) {
        printf("%f, ", v->array[sp_i]);
    }
    printf("\n");
    TEMPLATE(deleteVector,double)(v);
    free(actors);
    return 0;
}


void *main_func(void *args) {
    struct actor_main *main_s = (struct actor_main*)args;
    if(main_s->argc>1) {
        if(strcmp(main_s->argv[1], "async") == 0) {
            spawn_actor(disp_run, NULL);
        } else
        if(strcmp(main_s->argv[1], "sync") == 0) {
            for(size_t t=0; t<TMAX; t++) {
                for(size_t ai=0; ai<NUM_ACTORS; ai++) {
                    double a = calc_func(t);
                }
            }
        }
    }
    printf("out\n");
    return 0;
}	

DECLARE_ACTOR_MAIN(main_func)
