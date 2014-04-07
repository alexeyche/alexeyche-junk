#define _POSIX_C_SOURCE 200809L


#include "core.h"
#include "arg_opt.h"

#include "sim.h"
#include "io.h"




int main(int argc, char **argv) {
	
	ArgOptions a = parseOptions(argc, argv);
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
    if(a.input_spikes_file) {
        c->input_spikes_filename = strdup(a.input_spikes_file);
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
    configreNetSpikesSim(s, c);
//    printSRMLayer(s->layers->array[0]);
//    printSpikesList(s->ns->net);
//    printConnMap(s->ns);
//    printInputSpikesQueue(s->ns);
    runSim(s);
    
    if(saveStat) {
        pMatrixVector *mv = TEMPLATE(createVector,pMatrix)();
        Matrix *mp = vectorArrayToMatrix(s->layers->array[0]->stat_p, s->layers->array[0]->N);
        TEMPLATE(insertVector,pMatrix)(mv, mp);
        Matrix *mu = vectorArrayToMatrix(s->layers->array[0]->stat_u, s->layers->array[0]->N);
        TEMPLATE(insertVector,pMatrix)(mv, mu);
        Matrix *mB = vectorArrayToMatrix(s->layers->array[0]->stat_B, s->layers->array[0]->N);
        TEMPLATE(insertVector,pMatrix)(mv, mB);
        for(size_t ni=0; ni < s->layers->array[0]->N; ni++) {
            Matrix *mSyn = vectorArrayToMatrix(s->layers->array[0]->stat_W[ni], s->layers->array[0]->nconn[ni]);
            TEMPLATE(insertVector,pMatrix)(mv, mSyn);
        }
        for(size_t ni=0; ni < s->layers->array[0]->N; ni++) {
            Matrix *mC = vectorArrayToMatrix(s->layers->array[0]->stat_C[ni], s->layers->array[0]->nconn[ni]);
            TEMPLATE(insertVector,pMatrix)(mv, mC);
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
