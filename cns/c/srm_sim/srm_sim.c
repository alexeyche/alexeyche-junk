
#include "core.h"





void *main_func(void *args) {
    struct actor_main *main_s = (struct actor_main*)args;
    int x;
	
	/* Accessing the arguments passed to the application */
	printf("Number of arguments: %d\n", main_s->argc);
	for(x = 0; x < main_s->argc; x++) printf("Argument: %s\n", main_s->argv[x]);
	
    SRMLayer *l = createSRMLayer(30, 5);
    printSRMLayer(l);
    deleteSRMLayer(l);
}

DECLARE_ACTOR_MAIN(main_func)
