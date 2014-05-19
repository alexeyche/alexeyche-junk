
#include <core.h>

#include <args/postproc_opts.h>
#include <util/io.h>
#include <util/spikes_list.h>
#include <util/matrix.h>

int main(int argc, char **argv) {
    ArgOptionsPostProc a = parsePostProcOptions(argc, argv);
    pMatrixVector *input_struct = readMatrixList(a.input_spikes);
    
    assert(input_struct->size > 3);
    
    Matrix *spikes_m = input_struct->array[0];
    SpikesList *spikes = spikesMatrixToSpikesList(spikes_m);
    Matrix *timeline_m = input_struct->array[1];
    Matrix *classes_m = input_struct->array[2];
    doubleVector *timeline = TEMPLATE(copyFromArray,double)(timeline_m->vals, timeline_m->nrow*timeline_m->ncol);

    double ksize = a.kernel_values->array[0];

    pMatrixVector *hists = TEMPLATE(createVector,pMatrix)();
    double t;
    size_t i;

    size_t spike_indices[spikes->size];
    for(size_t ni=0; ni < spikes->size; ni++) {
        spike_indices[ni] = 0;
    }
    double spike_per_cell = 1.0/ksize;

    for(t=0, i=0; t < timeline->size * a.dur; t+=a.dur, i++) {
        Matrix *hist = createZeroMatrix(spikes->size, a.dur / ksize);
        for(size_t ni=0; ni < spikes->size; ni++) {
            if(spike_indices[ni] < spikes->list[ni]->size) {
                double sp_t = spikes->list[ni]->array[ spike_indices[ni] ];
                while((sp_t > t)&&(sp_t <= t+a.dur)) {
                    size_t hist_index = (sp_t -t)*ksize;
                    double v = getMatrixElement(hist, ni, hist_index);
                    setMatrixElement(hist, ni, hist_index, v + spike_per_cell);
                    spike_indices[ni] += 1;
                    if(spike_indices[ni] >= spikes->list[ni]->size) break;
                }
            }
        }
        TEMPLATE(insertVector,pMatrix)(hists, hist);
    }
    
    TEMPLATE(deleteVector,double)(timeline);
    TEMPLATE(deleteVector,double)(a.kernel_values);
    TEMPLATE(deleteVector,pMatrix)(input_struct);
    deleteSpikesList(spikes);
    return(0);
}
 
