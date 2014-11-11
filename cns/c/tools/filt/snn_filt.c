

#include <core/core.h>


#include <core/util/spikes_list.h>
#include <core/util/io.h>
#include <core/util/util.h>

#include "filt_funcs.h"
#include "optim_lbfgs.h"
#include <lbfgs.h>


#include "filt_opts.h"

int main(int argc, char **argv) {
    srand(time(NULL));

    ArgOptionsFilt a = parseFiltOptions(argc, argv);
    pMatrixVector* ts_data = readMatrixListFromFile(a.input_file);
    pMatrixVector* target_data = readMatrixListFromFile(a.target_values_file);
    assert(ts_data->size > 0);
    assert(target_data->size > 0);

    Matrix *spike_data = ts_data->array[0];
    doubleVector *target = doubleVectorFromMatrix(target_data->array[0]);
    assert(spike_data->ncol == target->size);

    Matrix *w_opt = runLbfgsOptim(spike_data, target, a.filter_size, a.jobs, a.epsilon);

    pMatrixVector *out = TEMPLATE(createVector,pMatrix)();
    TEMPLATE(insertVector,pMatrix)(out, w_opt);
    saveMatrixListToFile(a.output_file, out);

    TEMPLATE(deleteVector,pMatrix)(ts_data);
    TEMPLATE(deleteVector,pMatrix)(out);
    return 0;
} 
