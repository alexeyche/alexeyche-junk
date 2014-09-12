#include <core.h>

#include <args/filt_opts.h>

#include <util/spikes_list.h>
#include <util/io.h>
#include <util/util.h>

int main(int argc, char **argv) {
    ArgOptionsFilt a = parseFiltOptions(argc, argv);
    pMatrixVector* ts_data = readMatrixListFromFile(a.input_file);
    assert(ts_data->size > 0);
    int M = ts_data->array[0]->nrow;

    return 0;
} 
