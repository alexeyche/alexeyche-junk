#include <core/core.h>
#include <time.h>


#include <core/util/spikes_list.h>
#include <core/util/io.h>
#include <core/util/util.h>

#include <core/constants.h>

#include "prep_opts.h"
#include "run.h"

int main(int argc, char **argv) {
    ArgOptionsPrep a = parsePrepOptions(argc, argv);
    Constants *c = createConstants(a.const_filename);
//    printConstants(c);

    pMatrixVector* ts_data = readMatrixListFromFile(a.input_file);
    assert(ts_data->size > 0);

    pMatrixVector* ts_labels_m = readMatrixListFromFile(a.input_labels_file);
    assert(ts_labels_m->size == 1);
    assert(ts_labels_m->array[0]->nrow == ts_data->size);

    doubleVector *ts_labels = doubleVectorFromMatrix(ts_labels_m->array[0]);

    SpikePatternsList *spl = runNeurons(ts_data, ts_labels, c, false);
    saveSpikePatternsListToFile(spl, a.output_file);
     
    return(0.0);
} 
