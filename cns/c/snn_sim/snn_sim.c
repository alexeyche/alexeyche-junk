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
    for(size_t i=0; i<c->lc->size; i++) {
        if(a.learn == 0) {
            getLayerConstantsC(c,i)->learn = false;
        }
    }
    printConstants(c);
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
//    LayerPoisson *l = s->layers->array[0];
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
        FileStream *fs = createOutputFileStream(a.stat_file);

        for(size_t li=0; li < s->layers->size; li++) {
            LayerPoisson *l = s->layers->array[li];
            l->saveStat(l, fs);
        }            
        deleteFileStream(fs);
    }
    char *model_to_save = NULL;
    if(a.model_file) model_to_save = strdup(a.model_file);
    if(a.model_file_save) model_to_save = strdup(a.model_file_save); 
    
    if(model_to_save) { 
        saveLayersToFile(s, model_to_save);
    }
    if(a.output_spikes_file) {
        pMatrixVector *mv = TEMPLATE(createVector,pMatrix)();
        
        TEMPLATE(insertVector,pMatrix)(mv, vectorArrayToMatrix(s->ns->net->list, s->ns->net->size));
        TEMPLATE(insertVector,pMatrix)(mv, vectorArrayToMatrix(&s->rt->reset_timeline, 1));
        TEMPLATE(insertVector,pMatrix)(mv, vectorArrayToMatrix(&s->rt->pattern_classes, 1));

        checkIdxFnameOfFile(a.output_spikes_file);
        saveMatrixListToFile(a.output_spikes_file, mv);

        TEMPLATE(deleteVector,pMatrix)(mv);
    }
    deleteSim(s);
    deleteConstants(c);
}
