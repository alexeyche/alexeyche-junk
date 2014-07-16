#define _POSIX_C_SOURCE 200809L


#include <core.h>
#include <args/sim_opts.h>

#include <sim/sim.h>
#include <sim/serialize.h>
#include <util/io.h>




int main(int argc, char **argv) {
	
	ArgOptionsSim a = parseSimOptions(argc, argv);
//    printArgs(&a);
    bool saveStat = a.stat_file != NULL;
    Constants *c = createConstants(a.const_filename);
    srand(a.seed);
//    printConstants(c);
    for(size_t i=0; i<c->lc->size; i++) {
        if(a.learn == 0) {
            getLayerConstantsC(c,i)->learn = false;
            getLayerConstantsC(c,i)->learning_rule = ENull;
        } else
        if(a.learn == 1) {
            getLayerConstantsC(c,i)->learn = true;
        }
    }
    assert(a.jobs != 0);
    unsigned char statLevel = 0;
    if(saveStat) {
        statLevel = a.statLevel;
        if(a.calcStat) {
            statLevel = 1;
        }
    }        
    Sim *s = createSim(a.jobs, statLevel, c);
    
    char *model_to_load = NULL;
    if(a.model_file) model_to_load = strdup(a.model_file);
    if(a.model_file_load) model_to_load = strdup(a.model_file_load); 
    
    configureLayersSim(s, c);

    if(model_to_load) {
        loadLayersFromFile(s, model_to_load);
    }

    configureNetSpikesSim(s, a.input_spikes_file, c);
    configureSynapses(s, c);
    if(a.Tmax > 0) {
        s->rt->Tmax = a.Tmax;
    }
//    Layer *l = s->layers->array[0];
//    l->printLayer(l);
//    printSpikesList(s->ns->net);
//    printConnMap(s->ns);
//    printInputSpikesQueue(s->ns);
//    return(0);
    runSim(s);
    
    if(a.input_port>0) {
        printf("Not implemented\n");        
    }

    if(statLevel > 0) {
        checkIdxFnameOfFile(a.stat_file);
        pMatrixVector *mv = TEMPLATE(createVector,pMatrix)();
        if((c->reinforcement)&&(!a.calcStat)) {
            Matrix *m_stat_glob_rew = vectorArrayToMatrix(&s->ctx->stat_global_reward, 1);
            TEMPLATE(insertVector,pMatrix)(mv, m_stat_glob_rew);
        }
        for(size_t li=0; li < s->layers->size; li++) {
            Layer *l = s->layers->array[li];
            if(a.calcStat) {
                indVector *active_neurons = TEMPLATE(createVector,ind)();
                for(size_t ni=0; ni<l->N; ni++) {
                    TEMPLATE(insertVector,ind)(active_neurons, ni);
                }
                assert(l->N > 0);
                Matrix *mp = createMatrix(active_neurons->size, l->stat->stat_p[0]->size);
                Matrix *mf = createMatrix(active_neurons->size, l->stat->stat_fired[0]->size);
                assert(mp->ncol == mf->ncol);
                for(size_t ni=0; ni < active_neurons->size; ni++) {
                    for(size_t el_i=0; el_i < mp->ncol; el_i++) {
                        setMatrixElement(mp, ni, el_i, l->stat->stat_p[ active_neurons->array[ni] ]->array[ el_i] );
                        setMatrixElement(mf, ni, el_i, l->stat->stat_fired[ active_neurons->array[ni] ]->array[ el_i] );
                    }
                }
                TEMPLATE(insertVector,pMatrix)(mv, mp);
                TEMPLATE(insertVector,pMatrix)(mv, mf);
                Matrix *m_patt_classes = vectorArrayToMatrix(&s->rt->pattern_classes, 1);
                TEMPLATE(insertVector,pMatrix)(mv, m_patt_classes);
            } else {
                Matrix *mp = vectorArrayToMatrix(l->stat->stat_p, l->N);
                TEMPLATE(insertVector,pMatrix)(mv, mp);
            }
            if(statLevel > 1) {
                Matrix *mu = vectorArrayToMatrix(l->stat->stat_u, l->N);
                TEMPLATE(insertVector,pMatrix)(mv, mu);
                
                for(size_t ni=0; ni < l->N; ni++) {
                    Matrix *mSyn = vectorArrayToMatrix(l->stat->stat_syn[ni], l->nconn[ni]);
                    TEMPLATE(insertVector,pMatrix)(mv, mSyn);
                }
                for(size_t ni=0; ni < l->N; ni++) {
                    Matrix *mdW = vectorArrayToMatrix(l->stat->stat_W[ni], l->nconn[ni]);
                    TEMPLATE(insertVector,pMatrix)(mv, mdW);
                }
                if(l->ls_t) {
                    OptimalSTDP *ls = (OptimalSTDP*)l->ls_t;
                    Matrix *mB = vectorArrayToMatrix(ls->stat_B, l->N);

                    TEMPLATE(insertVector,pMatrix)(mv, mB);
                    for(size_t ni=0; ni < l->N; ni++) {
                        Matrix *mC = vectorArrayToMatrix(ls->stat_C[ni], l->nconn[ni]);
                        TEMPLATE(insertVector,pMatrix)(mv, mC);
                    }
                }                    
            }
        }            
     
        saveMatrixListToFile(a.stat_file, mv);
            
        TEMPLATE(deleteVector,pMatrix)(mv);
    }
    char *model_to_save = NULL;
    if(a.model_file) model_to_save = strdup(a.model_file);
    if(a.model_file_save) model_to_save = strdup(a.model_file_save); 
    
    if(model_to_save) { 
        saveLayersToFile(s, model_to_save);
    }
    if(a.output_spikes_file) {
        pMatrixVector *mv = TEMPLATE(createVector,pMatrix)();
        
        Matrix *spikes = vectorArrayToMatrix(s->ns->net->list, s->ns->net->size);
        TEMPLATE(insertVector,pMatrix)(mv, spikes);
        
        Matrix *timeline = vectorArrayToMatrix(&s->rt->reset_timeline, 1);
        TEMPLATE(insertVector,pMatrix)(mv, timeline);

        Matrix *classes  = vectorArrayToMatrix(&s->rt->pattern_classes, 1);
        TEMPLATE(insertVector,pMatrix)(mv, classes);

        saveMatrixListToFile(a.output_spikes_file, mv);

        TEMPLATE(deleteVector,pMatrix)(mv);
    }
    deleteSim(s);
    deleteConstants(c);
}
