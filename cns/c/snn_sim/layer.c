

Layer* createLayer(size_t N, size_t *glob_idx, unsigned char statLevel) {
    Layer *l = (Layer*)malloc(sizeof(SRMLayer));
    l->N = N;
    l->ids = (size_t*)malloc( l->N*sizeof(size_t));
    l->nt = (nspec_t*)malloc( l->N*sizeof(nspec_t));
    for(size_t ni=0; ni<l->N; ni++) {
        l->ids[ni] = (*glob_idx)++;
    }

    l->W = (double**)malloc( l->N*sizeof(double*));
    l->u = (double*)malloc( l->N*sizeof(double));
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
    l->calculateMembranePotentials = &calculateMembranePotentials_default;
    return(l);
}

void deleteLayer(Layer *l) {
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

void calculateMembranePotentials_default(Layer *l, const size_t *ni, const Constants *s, const double *t) {
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

void calculateSpike_default(struct Layer *l, size_t *ni) {
    double p = probf(&l->u[*ni], c) * c->dt;
    double coin = getUnif();
    if( p > coin ) {
        l->fired[ *ni ] = 1;
    }
    if(c->learn) {
        l->ls_t->trainWeightsStep(l->ls_t, &u, &p, &M, ni, s);
    }
}

