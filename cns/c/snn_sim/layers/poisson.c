
#include <sim/sim.h>

#include <util/templates_clean.h>
#define T pLayer
#define DESTRUCT_METHOD deleteLayer
#include <util/util_vector_tmpl.c>

LayerPoisson* createPoissonLayer(size_t N, size_t *glob_idx, unsigned char statLevel) {
    LayerPoisson *l = (LayerPoisson*)malloc(sizeof(LayerPoisson));
    l->N = N;
    l->ids = (size_t*)malloc( l->N*sizeof(size_t));
    l->nt = (nspec_t*)malloc( l->N*sizeof(nspec_t));
    for(size_t ni=0; ni<l->N; ni++) {
        l->ids[ni] = (*glob_idx)++;
    }

    l->W = (double**)malloc( l->N*sizeof(double*));
    l->u = (double*)malloc( l->N*sizeof(double));
    l->p = (double*)malloc( l->N*sizeof(double));
    l->M = (double*) malloc( l->N*sizeof(double));
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
    l->gr = (double*)malloc( l->N*sizeof(double));

    l->stat = (LayerStat*)malloc(sizeof(LayerStat));
    l->stat->statLevel = statLevel;
    if(l->stat->statLevel > 0) {
        l->stat->stat_p = (doubleVector**) malloc( l->N*sizeof(doubleVector*));
        l->stat->stat_fired = (doubleVector**) malloc( l->N*sizeof(doubleVector*));
        l->stat->stat_u = (doubleVector**) malloc( l->N*sizeof(doubleVector*));
        for(size_t ni=0; ni<l->N; ni++) {
            l->stat->stat_p[ni] = TEMPLATE(createVector,double)();
            l->stat->stat_fired[ni] = TEMPLATE(createVector,double)(); 
            l->stat->stat_u[ni] = TEMPLATE(createVector,double)();
        }
        if(l->stat->statLevel > 1) {
            l->stat->stat_W = (doubleVector***) malloc( l->N*sizeof(doubleVector**));
            l->stat->stat_syn = (doubleVector***) malloc( l->N*sizeof(doubleVector**));
        }
    }

    l->ls_t = NULL;
    l->calculateProbability = &calculateProbability_Poisson;
    l->calculateSpike = &calculateSpike_Poisson;
    l->calculateDynamics = &calculateDynamics_Poisson;
    l->deleteLayer = &deleteLayer_Poisson;
    l->configureLayer = &configureLayer_Poisson;
    l->toStartValues = &toStartValues_Poisson;
    l->propagateSpike = &propagateSpike_Poisson;
    l->allocSynData = &allocSynData_Poisson;
    l->deallocSynData = &deallocSynData_Poisson;
    l->printLayer = &printLayer_Poisson;
    l->serializeLayer = &serializeLayer_Poisson;
    l->deserializeLayer= &deserializeLayer_Poisson;
    l->saveStat = &saveStat_Poisson;
    return(l);
}

void deleteLayer_Poisson(LayerPoisson *l) {
    for(size_t ni=0; ni<l->N; ni++) {

        TEMPLATE(deleteLList,ind)(l->active_syn_ids[ni]);
        if(l->stat->statLevel > 0) {
            TEMPLATE(deleteVector,double)(l->stat->stat_p[ni]);
            TEMPLATE(deleteVector,double)(l->stat->stat_fired[ni]);
            TEMPLATE(deleteVector,double)(l->stat->stat_u[ni]);
        }
    }
    l->deallocSynData(l);
    if(l->ls_t) {
        l->ls_t->free(l->ls_t);
    }
    free(l->u); 
    free(l->p); 
    free(l->M);
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
    free(l->gr);
    if(l->stat->statLevel >0) {    
        free(l->stat->stat_p);
        free(l->stat->stat_fired);
        free(l->stat->stat_u);
    }
    free(l);
}

const LayerConstants* getLC(LayerPoisson *l, const Constants *c) {
    return( c->lc->array[l->id] );
}

size_t getLocalNeuronId(LayerPoisson *l, const size_t *glob_id) {
    int local_id = *glob_id - l->ids[0];
    if((local_id<0)||(local_id>=l->N)) printf("Error: Can't find neuron with id %zu\n", *glob_id);
    return(local_id);
}

const size_t getGlobalId(LayerPoisson *l, const size_t *ni) {
    return(l->ids[*ni]);
}

double getSynDelay(LayerPoisson *l, const size_t *ni, const size_t *syn_id) {
    assert(*syn_id < l->nconn[*ni]);
    return(l->syn_del[*ni][*syn_id]);

}


void setSynapseSpeciality(LayerPoisson *l, size_t ni, size_t syn_id, double spec) {
    l->syn_spec[ni][syn_id] = spec;
}



void initLearningRule(LayerPoisson *l, const Constants *c) {
    if( getLC(l,c)->learning_rule == EOptimalSTDP) {
        l->ls_t = (learn_t*)init_OptimalSTDP(l);
    } else
    if( getLC(l,c)->learning_rule == EResourceSTDP) {
        l->ls_t = (learn_t*)init_TResourceSTDP(l);
    } else
    if( getLC(l,c)->learning_rule == ESimpleSTDP) {
        l->ls_t = (learn_t*)init_SimpleSTDP(l);
    } else
    if( getLC(l,c)->learning_rule == ETripleSTDP) {
        l->ls_t = (learn_t*)init_TripleSTDP(l);
    } 
}

void initProbFun(LayerPoisson *l, const Constants *c) {
    if( getLC(l,c)->prob_fun == EExp) {
        l->prob_fun = &prob_fun_Exp;
        l->prob_fun_stroke = &prob_fun_stroke_Exp;
    } else
    if( getLC(l,c)->prob_fun == EExpHennequin) {
        l->prob_fun = &prob_fun_ExpHennequin;
        l->prob_fun_stroke = &prob_fun_stroke_ExpHennequin;
    } else
    if( getLC(l,c)->prob_fun == EExpBohte) {
        l->prob_fun = &prob_fun_ExpBohte;
        l->prob_fun_stroke = &prob_fun_stroke_ExpBohte;
    } else
    if( getLC(l,c)->prob_fun == ELinToyoizumi) {
        l->prob_fun = &prob_fun_LinToyoizumi;
        l->prob_fun_stroke = &prob_fun_stroke_LinToyoizumi;
    } 
}

void configureLayer_Poisson(LayerPoisson *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c) {
    indVector **layer_conns = (indVector**) malloc(l->N * sizeof(indVector));
    double acc_prob_net_edge = getLC(l,c)->net_edge_prob0;
    double delta_prob_net_edge = (getLC(l,c)->net_edge_prob1 - getLC(l,c)->net_edge_prob0)/(l->N/getLC(l,c)->net_edge_prob_group_size);
    double acc_prob_input_edge = getLC(l,c)->input_edge_prob0;
    double delta_prob_input_edge = (getLC(l,c)->input_edge_prob1 - getLC(l,c)->input_edge_prob0)/(l->N/getLC(l,c)->input_edge_prob_group_size);
    
    for(size_t ni=0; ni<l->N; ni++) {
        layer_conns[ni] = TEMPLATE(createVector,ind)();
        l->nt[ni] = EXC;
        if (getLC(l,c)->inhib_frac > getUnif()) {
            l->nt[ni] = INH;
        }
        for(size_t nj=0; nj<l->N; nj++) {        
            if(ni != nj) {
                if(acc_prob_net_edge > getUnif()) {
                    TEMPLATE(insertVector,ind)(layer_conns[ni], l->ids[nj]);
                }
            }
        }
        if(inputIDs) {
            for(size_t inp_i=0; inp_i<inputIDs->size; inp_i++) {        
                if(acc_prob_input_edge > getUnif()) {
                    TEMPLATE(insertVector,ind)(layer_conns[ni], inputIDs->array[inp_i]);
                }
            }
        }
        if(outputIDs) {
            for(size_t outp_i=0; outp_i<outputIDs->size; outp_i++) {        
                if(getLC(l,c)->output_edge_prob > getUnif()) {
                    TEMPLATE(insertVector,ind)(layer_conns[ni], outputIDs->array[outp_i]);
                }
            }
        }
        l->nconn[ni] = layer_conns[ni]->size; 
        if(ni % getLC(l,c)->net_edge_prob_group_size == 0) {
            acc_prob_net_edge += delta_prob_net_edge;
        }
        if(ni % getLC(l,c)->input_edge_prob_group_size == 0) {
            acc_prob_input_edge += delta_prob_input_edge;
        }
    }
    
    l->allocSynData(l);
    for(size_t ni=0; ni<l->N; ni++) {
        if(l->nconn[ni]>0) {
            memcpy(l->id_conns[ni], layer_conns[ni]->array, l->nconn[ni]*sizeof(size_t));
        }
        TEMPLATE(deleteVector,ind)(layer_conns[ni]);
    }
    free(layer_conns);
    // allocation done
    // configure learning part 
    initProbFun(l, c);
    if(getLC(l,c)->learn) {
        initLearningRule(l, c);
    }
    // start values assignment 
    l->toStartValues(l, c);
}

void toStartValues_Poisson(LayerPoisson *l, const Constants *c) {
    for(size_t ni=0; ni<l->N; ni++) {
        double start_weight = getLC(l,c)->weight_var * getNorm() + getLC(l,c)->weight_per_neuron/l->nconn[ni];
        l->fired[ni] = 0;
        if(getLC(l,c)->axonal_delays_rate > 0) {
            l->axon_del[ni] = getLC(l,c)->axonal_delays_gain*getExp(getLC(l,c)->axonal_delays_rate);
        } else {
            l->axon_del[ni] = 0.0;
        }
        l->gr[ni] = 0.0;
        l->M[ni] = 0.0;
        for(size_t syn_i=0; syn_i<l->nconn[ni]; syn_i++) {
            l->W[ni][syn_i] = start_weight;
            l->syn[ni][syn_i] = 0;
            l->syn_fired[ni][syn_i] = 0;
            l->syn_spec[ni][syn_i] = c->e_exc;
            if(getLC(l,c)->syn_delays_rate>0) {
                l->syn_del[ni][syn_i] = getLC(l,c)->syn_delays_gain*getExp(getLC(l,c)->syn_delays_rate);
            } else {
                l->syn_del[ni][syn_i] = 0.0;
            }
        }
    }        
    if(l->ls_t) {
        l->ls_t->toStartValues(l->ls_t);
    }
}


void propagateSpike_Poisson(LayerPoisson *l, const size_t *ni, const SynSpike *sp, const SimContext *s) {
    const Constants *c = s->c;
    if( ( l->syn[ *ni ][ sp->syn_id ] < SYN_ACT_TOL ) && ( l->syn[ *ni ][ sp->syn_id ] > -SYN_ACT_TOL )){
        TEMPLATE(addValueLList,ind)(l->active_syn_ids[*ni], sp->syn_id);
    } 
    l->syn[*ni][ sp->syn_id ] += l->syn_spec[*ni][ sp->syn_id ] * c->e0;
    l->syn_fired[*ni][ sp->syn_id ] = 1;
    if(l->ls_t) {
        l->ls_t->propagateSynSpike(l->ls_t, ni, sp, c);
    }
}

void calculateProbability_Poisson(LayerPoisson *l, const size_t *ni, const SimContext *s) {
    const Constants *c = s->c;
    l->u[*ni] = c->u_rest;

    indLNode *act_node;
    while( (act_node = TEMPLATE(getNextLList,ind)(l->active_syn_ids[ *ni ]) ) != NULL ) {
        const size_t *syn_id = &act_node->value;
        l->u[*ni] += l->W[ *ni ][ *syn_id ] * l->syn[ *ni ][ *syn_id ];
    }
    //// pacemaker
    //if((l->id == 0)&&(c->pacemaker->pacemaker_on)) {
    //    double u_p = c->pacemaker->amplitude + c->pacemaker->amplitude * sin(2*PI*c->pacemaker->frequency * *t/1000 - l->ids[*n_id] * c->pacemaker->cumulative_period_delta/1000);
    //    l->u[*n_id] += u_p;
    //}

    l->p[*ni] = l->prob_fun(&l->u[*ni], c) * c->dt;
    l->M[*ni] = exp(-l->gr[ *ni ]);
    l->p[*ni] *= l->M[*ni];

    if(l->stat->statLevel > 0)  {
        TEMPLATE(insertVector,double)(l->stat->stat_u[*ni], l->u[*ni]);
        if(l->stat->statLevel > 1)  {
            for(size_t con_i=0; con_i<l->nconn[*ni]; con_i++) {
                TEMPLATE(insertVector,double)(l->stat->stat_syn[*ni][con_i], l->syn[ *ni ][ con_i ]);
                TEMPLATE(insertVector,double)(l->stat->stat_W[*ni][con_i], l->W[ *ni ][ con_i ]);
            }
        }
    }
}

void calculateSpike_Poisson(struct LayerPoisson *l, const size_t *ni, const SimContext *s) {
    const Constants *c = s->c;

    if( l->p[*ni] > getUnif() ) {
        l->fired[ *ni ] = 1;
        l->gr[ *ni ] += c->qr;
    }
    if(l->stat->statLevel > 0)  {
        TEMPLATE(insertVector,double)(l->stat->stat_p[*ni], l->p[*ni]);
    }
}

void calculateDynamics_Poisson(LayerPoisson *l, const size_t *ni, const SimContext *s) {
    const Constants *c = s->c;

    // training
    if(l->ls_t) {
        l->ls_t->trainWeightsStep(l->ls_t, &l->u[*ni], &l->p[*ni], &l->M[*ni], ni, s);   
    }

    // melting      
    indLNode *act_node;
    while( (act_node = TEMPLATE(getNextLList,ind)(l->active_syn_ids[ *ni ]) ) != NULL ) {
        const size_t *syn_id = &act_node->value;

        l->syn[ *ni ][ *syn_id ] -= l->syn[ *ni ][ *syn_id ]/c->tm;
        if(l->syn_fired[ *ni ][ *syn_id ] == 1) {
            l->syn_fired[ *ni ][ *syn_id ] = 0; 
        }
        if( ( l->syn[ *ni ][ *syn_id ] < SYN_ACT_TOL ) && ( l->syn[ *ni ][ *syn_id ] > -SYN_ACT_TOL )){
            TEMPLATE(dropNodeLList,ind)(l->active_syn_ids[ *ni ], act_node);
        }
    }
    l->gr[ *ni ] += -l->gr[ *ni ]/c->tr;
}

void allocSynData_Poisson(LayerPoisson *l) {
    for(size_t ni=0; ni<l->N; ni++) {
        l->id_conns[ni] = (size_t*) malloc(l->nconn[ni]*sizeof(size_t));
        l->W[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        l->syn[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        l->syn_spec[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        l->syn_fired[ni] = (unsigned char*) malloc(l->nconn[ni]*sizeof(unsigned char));
        l->syn_del[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        if(l->stat->statLevel > 1) {
            l->stat->stat_W[ni] = (doubleVector**) malloc( l->nconn[ni]*sizeof(doubleVector*));
            l->stat->stat_syn[ni] = (doubleVector**) malloc( l->nconn[ni]*sizeof(doubleVector*));
            for(size_t syn_i=0; syn_i < l->nconn[ni]; syn_i++) {
                l->stat->stat_W[ni][syn_i] = TEMPLATE(createVector,double)();
                l->stat->stat_syn[ni][syn_i] = TEMPLATE(createVector,double)();
            }
        }
    }        
}

void deallocSynData_Poisson(LayerPoisson *l) {
    for(size_t ni=0; ni<l->N; ni++) {
        free(l->id_conns[ni]);
        free(l->W[ni]);
        free(l->syn[ni]);
        free(l->syn_spec[ni]);
        free(l->syn_fired[ni]);
        free(l->syn_del[ni]);
        if(l->stat->statLevel > 1) {
            for(size_t syn_i=0; syn_i < l->nconn[ni]; syn_i++) {
                TEMPLATE(deleteVector,double)(l->stat->stat_W[ni][syn_i]);
                TEMPLATE(deleteVector,double)(l->stat->stat_syn[ni][syn_i]);
            }
            free(l->stat->stat_W[ni]);
            free(l->stat->stat_syn[ni]);
        }
    }        
}

void printLayer_Poisson(LayerPoisson *l) {
    printf("Poisson LayerPoisson, size: %zu \n", l->N);
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

//========================= serialize stuff ==============================


void writeLayerSerializeInfo(LayerSerializeInfo info, FileStream *file) {
    fwrite(&info.neuron_type, sizeof(int), 1, file->fd);
    fwrite(&info.learning_rule, sizeof(int), 1, file->fd);
}

LayerSerializeInfo readLayerSerializeInfo(FileStream *file) {
    LayerSerializeInfo info;
    if(fread(&info.neuron_type, sizeof(int), 1, file->fd) == 0) {
        printf("Error while reading neuron type from file %s\n", file->fname);
        exit(1);
    }
    if(fread(&info.learning_rule, sizeof(int), 1, file->fd) == 0) {
        printf("Error while reading learning rule from file %s\n", file->fname);
        exit(1);
    }
    return(info); 
}

#define POISSON_LAYER_SERIALIZATION_SIZE 6
void deserializeLayer_Poisson(LayerPoisson *l, FileStream *file, const Sim *s) {
    const Constants *c  = s->ctx->c;
    LayerSerializeInfo info = readLayerSerializeInfo(file);
    if(info.neuron_type != getLC(l,c)->neuron_type) {
        printf("Can't read layer with different type from model %s\n", file->fname);
        exit(1);
    }

    l->deallocSynData(l); 
    if(l->ls_t) {
        l->ls_t->free(l->ls_t);
    }

    pMatrixVector* data = readMatrixList(file, POISSON_LAYER_SERIALIZATION_SIZE);
    Matrix *W = data->array[0];
    Matrix *conns = data->array[1];
    Matrix *nt = data->array[2];
    Matrix *ids = data->array[3];
    Matrix *axon_del = data->array[4];
    Matrix *syn_del = data->array[5];
    assert(l->N == W->nrow);

    doubleVector **W_vals = (doubleVector**) malloc( W->nrow * sizeof(doubleVector*));
    doubleVector **syn_del_vals = (doubleVector**) malloc( syn_del->nrow * sizeof(doubleVector*));
    indVector **id_conns_vals = (indVector**) malloc( W->nrow * sizeof(indVector*));
    
    for(size_t i=0; i<W->nrow; i++) {  // shape form and get the values
        W_vals[i] = TEMPLATE(createVector,double)();
        syn_del_vals[i] = TEMPLATE(createVector,double)();
        id_conns_vals[i] = TEMPLATE(createVector,ind)();
        for(size_t j=0; j<W->ncol; j++) {
            double w_ij = getMatrixElement(W, i, j);
            double syn_del_ij = getMatrixElement(syn_del, i, j);
            double conn_ij = getMatrixElement(conns, i, j);
            if(conn_ij>0) {
                TEMPLATE(insertVector,double)(W_vals[i], w_ij);
                TEMPLATE(insertVector,double)(syn_del_vals[i], syn_del_ij);
                TEMPLATE(insertVector,ind)(id_conns_vals[i], j);
            }
        }
        l->nconn[i] = W_vals[i]->size;
        if(getMatrixElement(nt, i, 0) == 0) {
            l->nt[i] = EXC;
        } else 
        if(getMatrixElement(nt, i, 0) == 1.0) {
            l->nt[i] = INH;
        } else {
            printf("Error in neuron type\n");
            exit(1);
        }
        l->ids[i] = getMatrixElement(ids, i, 0);    
    }
    l->allocSynData(l); 
    if(info.learning_rule != getLC(l,c)->learning_rule) {
        printf("Can't read layer with different learning rule type from model %s\n", file->fname);
        exit(1);
    }

    if(getLC(l,c)->learn) {
        initLearningRule(l, c);
    }
    l->toStartValues(l, c);
    for(size_t ni=0; ni<l->N; ni++) {  // apply values
        assert(l->nconn[ni] == W_vals[ni]->size);
        
        l->W[ni] = copyDoubleMem(W_vals[ni]);
        l->id_conns[ni] = copyIndMem(id_conns_vals[ni]);
        l->syn_del[ni] = copyDoubleMem(syn_del_vals[ni]);
        l->axon_del[ni] = getMatrixElement(axon_del, ni, 0);

        TEMPLATE(deleteVector,double)(W_vals[ni]);
        TEMPLATE(deleteVector,double)(syn_del_vals[ni]);
        TEMPLATE(deleteVector,ind)(id_conns_vals[ni]);
    }
    if(l->ls_t) {
        l->ls_t->deserialize( l->ls_t, file, s );
    }
    
    free(syn_del_vals);
    free(W_vals);
    free(id_conns_vals);
    TEMPLATE(deleteVector,pMatrix)(data);
}


void serializeLayer_Poisson(LayerPoisson *l, FileStream *file, const Sim *s) {
    const Constants *c = s->ctx->c;
    LayerSerializeInfo info;
    info.neuron_type= getLC(l,c)->neuron_type;
    info.learning_rule = getLC(l,c)->learning_rule;
    writeLayerSerializeInfo(info, file);

    pMatrixVector *data = TEMPLATE(createVector,pMatrix)();
    size_t max_conn_id = 0;
    for(size_t ni=0; ni< l->N; ni++) {
        for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
            if(max_conn_id < l->id_conns[ni][con_i]) {
                max_conn_id = l->id_conns[ni][con_i];
            }
        }

    }
    Matrix *W = createMatrix(l->N, max_conn_id+1);
    Matrix *conns = createMatrix(l->N, max_conn_id+1);
    Matrix *nt = createMatrix(l->N, 1);
    Matrix *ids = createMatrix(l->N, 1);
    Matrix *axon_del = createMatrix(l->N, 1);
    Matrix *syn_del = createMatrix(l->N, max_conn_id+1);
   
    for(size_t i=0; i<W->nrow; i++) {
        for(size_t j=0; j<W->ncol; j++) {
            setMatrixElement(W, i, j, 0);
            setMatrixElement(syn_del, i, j, 0);
            setMatrixElement(conns, i, j, 0);
        }
    }
    for(size_t ni=0; ni< l->N; ni++) {
        for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
            setMatrixElement(W, ni, l->id_conns[ni][con_i], l->W[ni][con_i]);
            setMatrixElement(syn_del, ni, l->id_conns[ni][con_i], l->syn_del[ni][con_i]);
            setMatrixElement(conns, ni, l->id_conns[ni][con_i], 1);
        }
        setMatrixElement(nt, ni, 0, l->nt[ni]);
        setMatrixElement(ids, ni, 0, l->ids[ni]);
        setMatrixElement(axon_del, ni, 0, l->axon_del[ni]);
    }
    
    TEMPLATE(insertVector,pMatrix)(data, W);
    TEMPLATE(insertVector,pMatrix)(data, conns);
    TEMPLATE(insertVector,pMatrix)(data, nt);
    TEMPLATE(insertVector,pMatrix)(data, ids);
    TEMPLATE(insertVector,pMatrix)(data, axon_del);
    TEMPLATE(insertVector,pMatrix)(data, syn_del);
    saveMatrixList(file, data);
    TEMPLATE(deleteVector,pMatrix)(data);
    
    if(l->ls_t) {
        l->ls_t->serialize(l->ls_t, file, s);
    }
}

void saveStat_Poisson(LayerPoisson *l, FileStream *file) {
    pMatrixVector *mv = TEMPLATE(createVector,pMatrix)();

    Matrix *mp = vectorArrayToMatrix(l->stat->stat_p, l->N);
    TEMPLATE(insertVector,pMatrix)(mv, mp);
    
    Matrix *mu = vectorArrayToMatrix(l->stat->stat_u, l->N);
    TEMPLATE(insertVector,pMatrix)(mv, mu);
    
    if(l->stat->statLevel > 1) {
        for(size_t ni=0; ni < l->N; ni++) {
            Matrix *mSyn = vectorArrayToMatrix(l->stat->stat_syn[ni], l->nconn[ni]);
            TEMPLATE(insertVector,pMatrix)(mv, mSyn);
        }
        for(size_t ni=0; ni < l->N; ni++) {
            Matrix *mdW = vectorArrayToMatrix(l->stat->stat_W[ni], l->nconn[ni]);
            TEMPLATE(insertVector,pMatrix)(mv, mdW);
        }
    }        

    saveMatrixList(file, mv);
    TEMPLATE(deleteVector,pMatrix)(mv);
    
    if(l->ls_t) {
        l->ls_t->saveStat(l->ls_t, file);
    }        
}
