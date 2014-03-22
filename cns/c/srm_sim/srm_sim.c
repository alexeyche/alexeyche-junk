
#include "core.h"
#include "arg_opt.h"


void *main_func(void *args) {
    struct actor_main *main_s = (struct actor_main*)args;
	
	ArgOptions a = parseOptions(main_s->argc, main_s->argv);
    printf("args.jobs = %d \n", a.jobs); 	
    printf("args.c = %s \n", a.const_filename); 	

    Constants *c = createConstants(a.const_filename);
    printConstants(c);


    SRMLayer *l = createSRMLayer(c->N);
    configureSRMLayer(l, c);
    printSRMLayer(l);

    deleteSRMLayer(l);
    deleteConstants(c);
}

DECLARE_ACTOR_MAIN(main_func)
