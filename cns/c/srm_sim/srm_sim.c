
#include "core.h"
#include "arg_opt.h"

#include "sim.h"


void *main_func(void *args) {
    struct actor_main *main_s = (struct actor_main*)args;
	
	ArgOptions a = parseOptions(main_s->argc, main_s->argv);
    printf("args.jobs = %d \n", a.jobs); 	
    printf("args.c = %s \n", a.const_filename); 	

    Constants *c = createConstants(a.const_filename);
    printConstants(c);

    Sim *s = createSim();
    printf("0net_size: %zu\n", s->n.net_size);
    configureSim(s, c);    

    deleteSim(s);
    deleteConstants(c);
}

DECLARE_ACTOR_MAIN(main_func)
