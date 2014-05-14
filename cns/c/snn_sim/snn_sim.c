#define _POSIX_C_SOURCE 200809L


#include <core.h>
#include <args/sim_opts.h>

#include <sim.h>
#include <util/io.h>




int main(int argc, char **argv) {
	
	ArgOptionsSim a = parseSimOptions(argc, argv);
//    printArgs(&a);
    bool saveStat = a.stat_file != NULL;
    Constants *c = createConstants(a.const_filename);
    srand(a.seed);
    if(a.learn == 0) {
        c->learn = false;
    } else
    if(a.learn == 1) {
        c->learn = true;
    }
//    printConstants(c);
    assert(a.jobs != 0);
    Sim *s = createSim(a.jobs);
    
    char *model_to_load = NULL;
    if(a.model_file) model_to_load = strdup(a.model_file);
    if(a.model_file_load) model_to_load = strdup(a.model_file_load); 
    
    unsigned char statLevel = 0;
    if(saveStat) {
        statLevel = 2;
        if(a.calcStat) {
            statLevel = 1;
        }
    }        
    if(model_to_load) {
        loadLayersFromFile(s, model_to_load, c, statLevel);
    } else {
        configureLayersSim(s, c, statLevel);
    }

    configureNetSpikesSim(s, a.input_spikes_file, c);
    configureSynapses(s, c);
//    printSRMLayer(s->layers->array[0]);
//    printSRMLayer(s->layers->array[1]);
//    printSpikesList(s->ns->net);
//    printConnMap(s->ns);
//    printInputSpikesQueue(s->ns);
    runSim(s);
    
    if(a.input_port>0) {
        printf("Not implemented\n");        
    }

    if(statLevel > 0) {
        pMatrixVector *mv = TEMPLATE(createVector,pMatrix)();
        for(size_t li=0; li < s->layers->size; li++) {
            SRMLayer *l = s->layers->array[li];
            Matrix *mp = vectorArrayToMatrix(l->stat_p, l->N);
            TEMPLATE(insertVector,pMatrix)(mv, mp);
            if(a.calcStat) {
                Matrix *mf = vectorArrayToMatrix(l->stat_fired, l->N);
                TEMPLATE(insertVector,pMatrix)(mv, mf);
                Matrix *m_patt_classes = vectorArrayToMatrix(&s->rt->pattern_classes, 1);
                TEMPLATE(insertVector,pMatrix)(mv, m_patt_classes);
            }
            if(statLevel > 1) {
                Matrix *mu = vectorArrayToMatrix(l->stat_u, l->N);
                TEMPLATE(insertVector,pMatrix)(mv, mu);
                
                TOptimalSTDP *ls = (TOptimalSTDP*)l->ls_t;
                Matrix *mB = vectorArrayToMatrix(ls->stat_B, l->N);

                TEMPLATE(insertVector,pMatrix)(mv, mB);
                
                for(size_t ni=0; ni < l->N; ni++) {
                    Matrix *mSyn = vectorArrayToMatrix(l->stat_syn[ni], l->nconn[ni]);
                    TEMPLATE(insertVector,pMatrix)(mv, mSyn);
                }
                for(size_t ni=0; ni < l->N; ni++) {
                    Matrix *mC = vectorArrayToMatrix(ls->stat_C[ni], l->nconn[ni]);
                    TEMPLATE(insertVector,pMatrix)(mv, mC);
                }
                for(size_t ni=0; ni < l->N; ni++) {
                    Matrix *mdW = vectorArrayToMatrix(l->stat_W[ni], l->nconn[ni]);
                    TEMPLATE(insertVector,pMatrix)(mv, mdW);
                }
            }
        }            
     
        saveMatrixList(a.stat_file, mv);
            
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

        saveMatrixList(a.output_spikes_file, mv);

        TEMPLATE(deleteVector,pMatrix)(mv);
    }
    deleteSim(s);
    deleteConstants(c);
}
