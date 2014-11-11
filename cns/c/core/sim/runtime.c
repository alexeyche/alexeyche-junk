
#include "runtime.h"


SimRuntime* createRuntime() {
    SimRuntime *rt = (SimRuntime*) malloc(sizeof(SimRuntime));
    rt->reset_timeline = TEMPLATE(createVector,double)();
    rt->pattern_classes = TEMPLATE(createVector,double)();
    rt->uniq_classes = TEMPLATE(createVector,int)();
    rt->classes_indices_train = TEMPLATE(createVector,ind)(); 

    rt->timeline_iter = 0;
    rt->Tmax = 0;
    return(rt);
}

void deleteRuntime(SimRuntime *sr) {
    TEMPLATE(deleteVector,double)(sr->reset_timeline);
    TEMPLATE(deleteVector,int)(sr->uniq_classes);
    TEMPLATE(deleteVector,ind)(sr->classes_indices_train);
    free(sr);
}


