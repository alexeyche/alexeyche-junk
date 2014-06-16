

Layer* createPoissonLayer(size_t N, size_t *glob_idx, unsigned char statLevel) {
    Layer *l = (Layer*)malloc(sizeof(SRMLayer));
    l->N = N;
    l->ids = (size_t*)malloc( l->N*sizeof(size_t));
    l->nt = (nspec_t*)malloc( l->N*sizeof(nspec_t));
    for(size_t ni=0; ni<l->N; ni++) {
        l->ids[ni] = (*glob_idx)++;
    }

    l->W = (double**)malloc( l->N*sizeof(double*));
    l->u = (double*)malloc( l->N*sizeof(double));
    l->p = (double*)malloc( l->N*sizeof(double));
    l->syn = (double**)malloc( l->N*sizeof(double*));
    l->syn_spec = (double**)malloc( l->N*sizeof(double*));
    l->id_conns = (size_t**)malloc( l->N*sizeof(size_t*));

    l->nconn = (int*)malloc( l->N*sizeof(int));
    for(size_t ni=0; ni<l->N; ni++) {
        l->nconn[ni] = 0;
    }
    l->active_syn_ids = (indLList**) malloc( l->N*sizeof(indLList*));
    for(size_t ni=0; ni<l->N; ni++) {
        l->active_syn_ids[ni] = TEMPLATE(createLList,ind)();
    }
    
    l->axon_del = (double*)malloc( l->N*sizeof(double));
    l->syn_del = (double**)malloc( l->N*sizeof(double));
    l->fired = (unsigned char*)malloc( l->N*sizeof(unsigned char));
    l->syn_fired = (unsigned char**)malloc( l->N*sizeof(unsigned char*));
    
    l->stat = (LayerStat*)malloc(sizeof(LayerStat));
    l->stat->statLevel = statLevel;
    if(l->stat->statLevel > 0) {
        l->stat->stat_p = (doubleVector**) malloc( l->stat->N*sizeof(doubleVector*));
        l->stat->stat_fired = (doubleVector**) malloc( l->stat->N*sizeof(doubleVector*));
        for(size_t ni=0; ni<l->stat->N; ni++) {
            l->stat->stat_p[ni] = TEMPLATE(createVector,double)();
            l->stat->stat_fired[ni] = TEMPLATE(createVector,double)(); 

        }
        if(l->stat->statLevel > 1) {
            l->stat->stat_u = (doubleVector**) malloc( l->stat->N*sizeof(doubleVector*));
            l->stat->stat_W = (doubleVector***) malloc( l->stat->N*sizeof(doubleVector**));
            l->stat->stat_syn = (doubleVector***) malloc( l->stat->N*sizeof(doubleVector**));
            for(size_t ni=0; ni<l->N; ni++) {
                l->stat->stat_u[ni] = TEMPLATE(createVector,double)();
            }
        }
    }
    l->ls_t = NULL;
    l->calculateMembranePotentials = &calculateMembranePotentials_Poisson;
    l->calculateSpike_Poisson = &calculateSpike_Poisson;
    l->deleteLayer = &deleteLayer_Poisson;
    l->configureLayer = &configureLayer_Poisson;
    l->toStartValues = &toStartValues_Poisson;
    l->propagateSpike = &propagateSpike_Poisson;
    l->allocSynData = &allocSynData_Poisson;
    l->printLayer = &printLayer_Poisson;
    return(l);
}

void deleteLayer_Poisson(Layer *l) {
    for(size_t ni=0; ni<l->N; ni++) {
        if(l->nconn[ni]>0) {
            free(l->W[ni]);
            free(l->syn[ni]);
            free(l->syn_spec[ni]);
            free(l->id_conns[ni]);
            free(l->syn_fired[ni]);
        }
        TEMPLATE(deleteLList,ind)(l->active_syn_ids[ni]);
        if(l->stat->statLevel > 0) {
            TEMPLATE(deleteVector,double)(l->stat->stat_p[ni]);
            TEMPLATE(deleteVector,double)(l->stat->stat_fired[ni]);
            if(l->stat->statLevel > 1) {
                TEMPLATE(deleteVector,double)(l->stat->stat_u[ni]);
                for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
                    TEMPLATE(deleteVector,double)(l->stat->stat_W[ni][con_i]);
                    TEMPLATE(deleteVector,double)(l->stat->stat_syn[ni][con_i]);
                }
            }                
        }
    }
    if(l->ls_t)
        l->ls_t->free(l->ls_t);
    free(l->u); 
    free(l->p); 
    free(l->ids);
    free(l->nt);
    free(l->id_conns);
    free(l->nconn);
    free(l->W);
    free(l->syn);
    free(l->syn_spec);
    free(l->active_syn_ids);
    free(l->fired);
    free(l->syn_fired);
    free(l->axon_del);
    if(l->stat->statLevel >0) {    
        free(l->stat->stat_p);
        free(l->stat->stat_fired);
        if(l->stat->statLevel > 1) {        
            free(l->stat->stat_u);
            free(l->stat->stat_W);
            free(l->stat->stat_syn);
        }            
    }
    free(l);
}

double layerConstD(Layer *l, doubleVector *v) {
    return( v->array[ l->id ]);
}

void calculateMembranePotentials_Poisson(Layer *l, const size_t *ni, const Constants *s, const double *t) {
    l->u[*ni] = c->u_rest;
    if((l->id == 0)&&(c->pacemaker->pacemaker_on)) {
        double u_p = c->pacemaker->amplitude + c->pacemaker->amplitude * sin(2*PI*c->pacemaker->frequency * *t/1000 - l->ids[*ni] * c->pacemaker->cumulative_period_delta/1000);
        l->u[*ni] += u_p;
    }
    indLNode *act_node;
    while( (act_node = TEMPLATE(getNextLList,ind)(l->active_syn_ids[ *ni ]) ) != NULL ) {
        const size_t *syn_id = &act_node->value;
        l->u[*ni] += l->W[ *ni ][ *syn_id ] * l->syn[ *ni ][ *syn_id ];
    }
}

void calculateSpike_Poisson(struct Layer *l, size_t *ni) {
    l->p[*ni] = probf(&l->u[*ni], c) * c->dt;
    double coin = getUnif();
    if( l->p[*ni] > coin ) {
        l->fired[ *ni ] = 1;
    }
    if(c->learn) {
        double M=0;
        l->ls_t->trainWeightsStep(l->ls_t, &l->u[*ni], &l->p[*ni], &M, ni, s);
    }
}

void initLearningRule(Layer *l, const Constants *c) {
    if( c->learning_rule == EOptimalSTDP) {
        l->ls_t = (learn_t*)init_TOptimalSTDP(l);
    } else
    if( c->learning_rule == EResourceSTDP) {
        l->ls_t = (learn_t*)init_TResourceSTDP(l);
    } 
}

void configureLayer_Poisson(Layer *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c) {
    indVector **layer_conns = (indVector**) malloc(l->N * sizeof(indVector));
    for(size_t ni=0; ni<l->N; ni++) {
        layer_conns[ni] = TEMPLATE(createVector,ind)(0);
        l->nt[ni] = EXC;
        if (c->inhib_frac->array[l->id] > getUnif()) {
            l->nt[ni] = INH;
        }
        for(size_t nj=0; nj<l->N; nj++) {        
            if(ni != nj) {
                if(c->net_edge_prob->array[l->id] > getUnif()) {
                    TEMPLATE(insertVector,ind)(layer_conns[ni], l->ids[nj]);
                }
            }
        }
        if(inputIDs) {
            for(size_t inp_i=0; inp_i<inputIDs->size; inp_i++) {        
                if(c->input_edge_prob->array[l->id] > getUnif()) {
                    TEMPLATE(insertVector,ind)(layer_conns[ni], inputIDs->array[inp_i]);
                }
            }
        }
        if(outputIDs) {
            for(size_t outp_i=0; outp_i<outputIDs->size; outp_i++) {        
                if(c->output_edge_prob->array[l->id] > getUnif()) {
                    TEMPLATE(insertVector,ind)(layer_conns[ni], outputIDs->array[outp_i]);
                }
            }
        }
        l->nconn[ni] = layer_conns[ni]->size; 
    }
    allocSynData(l);
    for(size_t ni=0; ni<l->N; ni++) {
        if(l->nconn[ni]>0) {
            memcpy(l->id_conns[ni], layer_conns[ni]->array, l->nconn[ni]*sizeof(size_t));
        }
        TEMPLATE(deleteVector,ind)(layer_conns[ni]);
    }
    // allocation done
    // configure learning part 
    initLearningRule(l, c)
    // start values assignment 
    l->toStartValues(l, c);

}

void toStartValues_Poisson(Layer *l, const Constants *c) {
    for(size_t ni=0; ni<l->N; ni++) {
        double start_weight = c->weight_var * getNorm() + c->weight_per_neuron->array[l->id]/l->nconn[ni];
        l->fired[ni] = 0;
        if(c->axonal_delays_rate > 0) {
            l->axon_del[ni] = c->axonal_delays_gain*getExp(c->axonal_delays_rate);
        } else {
            l->axon_del[ni] = 0.0;
        }
        for(size_t syn_i=0; syn_i<l->nconn[ni]; syn_i++) {
            l->W[ni][syn_i] = start_weight;
            l->syn[ni][syn_i] = 0;
            l->syn_fired[ni][syn_i] = 0;
            l->syn_spec[ni][syn_i] = c->e_exc;
            if(c->syn_delays_rate>0) {
                l->syn_del[ni][syn_i] = c->syn_delays_gain*getExp(c->syn_delays_rate);
            } else {
                l->syn_del[ni][syn_i] = 0.0;
            }
        }
    }        
    if(c->axonal_delays_rate > 0) {
        for(size_t ni=0; ni<l->N; ni++) {
            for(size_t syn_i=0; syn_i<l->nconn[ni]; syn_i++) {
            }
        }
    }

    if(l->ls_t) {
        l->ls_t->toStartValues(l->ls_t);
    }
}


void propagateSpike_Poisson(Layer *l, const size_t *ni, const SynSpike *sp, const Constants *c) {
    if(l->syn[*ni][ sp->syn_id ] < SYN_ACT_TOL) {
        TEMPLATE(addValueLList,ind)(l->active_syn_ids[*ni], sp->syn_id);
    } 
    
    l->syn[*ni][ sp->syn_id ] += l->syn_spec[*ni][ sp->syn_id ] * c->e0;
#if BACKPROP_POT == 1    
    l->syn[*ni][ sp->syn_id ] *= l->a[*ni];
#endif
    l->syn_fired[*ni][ sp->syn_id ] = 1;
    l->ls_t->propagateSynSpike(l->ls_t, ni, sp, c);
}


void allocSynData_Poisson(Layer *l) {
    for(size_t ni=0; ni<l->N; ni++) {
        l->id_conns[ni] = (size_t*) malloc(l->nconn[ni]*sizeof(size_t));
        l->W[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        l->syn[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        l->syn_spec[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        l->syn_fired[ni] = (unsigned char*) malloc(l->nconn[ni]*sizeof(unsigned char));
        l->syn_del[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        if(l->statLevel > 1) {
            l->stat_W[ni] = (doubleVector**) malloc( l->nconn[ni]*sizeof(doubleVector*));
            l->stat_syn[ni] = (doubleVector**) malloc( l->nconn[ni]*sizeof(doubleVector*));
            for(size_t syn_i=0; syn_i < l->nconn[ni]; syn_i++) {
                l->stat_W[ni][syn_i] = TEMPLATE(createVector,double)();
                l->stat_syn[ni][syn_i] = TEMPLATE(createVector,double)();
            }
        }
    }        
}


void printLayer_Poisson(Layer *l) {
    printf("SRMLayer, size: %zu \n", l->N);
    for(size_t ni=0; ni<l->N; ni++) {
        printf("%zu, ", l->ids[ni]);
        if(l->nt[ni] == EXC) {
            printf("excitatory, ");
        } else 
        if(l->nt[ni] == INH) {
            printf("inhibitory, ");
        }
        printf("connected with: ");
        for(size_t syn_i=0; syn_i<l->nconn[ni]; syn_i++) {
            char *spec;
            if(l->syn_spec[ni][syn_i] > 0) {
                spec = strdup("exc");
            } else 
            if(l->syn_spec[ni][syn_i] < 0) {
                spec = strdup("inh");
            } else {
                printf("Something wrong!\n");
                assert(0);
            }
            printf("%zu:%f (%s),", l->id_conns[ni][syn_i], l->W[ni][syn_i], spec);
            free(spec);
        }
        printf("\n");
    }
}


