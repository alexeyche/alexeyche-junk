#define _POSIX_C_SOURCE 200809L


#include "core.h"
#include "arg_opt.h"

#include "sim.h"
#include "io.h"



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
    
    if(model_to_load) {
        loadLayersFromFile(s, model_to_load, c, saveStat);        
    } else {
        configureLayersSim(s, c, saveStat);    
    }

    configureNetSpikesSim(s, a.input_spikes_file, c);

    configureSynapses(s, c);
//    printSRMLayer(s->layers->array[0]);
//    printSRMLayer(s->layers->array[1]);
//    printSpikesList(s->ns->net);
//    printConnMap(s->ns);
//    printInputSpikesQueue(s->ns);
    if(a.Tmax > 0) {
        s->rt->Tmax = a.Tmax;
    }
    runSim(s);
    
    if(a.input_port>0) {
        printf("Not implemented\n");        
    }

    if(saveStat) {
        pMatrixVector *mv = TEMPLATE(createVector,pMatrix)();
        for(size_t li=0; li < s->layers->size; li++) {
            SRMLayer *l = s->layers->array[li];
            Matrix *mp = vectorArrayToMatrix(l->stat_p, l->N);
            TEMPLATE(insertVector,pMatrix)(mv, mp);
            Matrix *mu = vectorArrayToMatrix(l->stat_u, l->N);
            TEMPLATE(insertVector,pMatrix)(mv, mu);
            
            if(c->learning_rule == EOptimalSTDP) {
                TOptimalSTDP *ls = (TOptimalSTDP*)l->ls_t;
                Matrix *mB = vectorArrayToMatrix(ls->stat_B, l->N);
                TEMPLATE(insertVector,pMatrix)(mv, mB);

                for(size_t ni=0; ni < l->N; ni++) {
                    Matrix *mC = vectorArrayToMatrix(ls->stat_C[ni], l->nconn[ni]);
                    TEMPLATE(insertVector,pMatrix)(mv, mC);
                }
            } else
            if(c->learning_rule == EResourceSTDP) {
                TResourceSTDP *ls = (TResourceSTDP*)l->ls_t;
                Matrix *m_res = vectorArrayToMatrix(ls->stat_res, l->N);
                TEMPLATE(insertVector,pMatrix)(mv, m_res);

                Matrix *m_y_tr = vectorArrayToMatrix(ls->stat_y_tr, l->N);
                TEMPLATE(insertVector,pMatrix)(mv, m_y_tr);

                for(size_t ni=0; ni < l->N; ni++) {
                    Matrix *m_x_tr = vectorArrayToMatrix(ls->stat_x_tr[ni], l->nconn[ni]);
                    TEMPLATE(insertVector,pMatrix)(mv, m_x_tr);
                }
            }
            for(size_t ni=0; ni < l->N; ni++) {
                Matrix *mSyn = vectorArrayToMatrix(l->stat_syn[ni], l->nconn[ni]);
                TEMPLATE(insertVector,pMatrix)(mv, mSyn);
            }
            
            for(size_t ni=0; ni < l->N; ni++) {
                Matrix *mdW = vectorArrayToMatrix(l->stat_W[ni], l->nconn[ni]);
                TEMPLATE(insertVector,pMatrix)(mv, mdW);
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
        Matrix *spikes = vectorArrayToMatrix(s->ns->net->list, s->ns->net->size);
        pMatrixVector *mv = TEMPLATE(createVector,pMatrix)();
        TEMPLATE(insertVector,pMatrix)(mv, spikes);

        saveMatrixList(a.output_spikes_file, mv);

        TEMPLATE(deleteVector,pMatrix)(mv);
    }
    deleteSim(s);
    deleteConstants(c);
}
