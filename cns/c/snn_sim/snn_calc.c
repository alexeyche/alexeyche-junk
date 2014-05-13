

#include <core.h>
#include <args/calc_opts.h>

#include <util/io.h>
#include <util/matrix.h>
#include <math.h>

#include <calc.h>

int main(int argc, char **argv) {
	ArgOptionsCalc a = parseCalcOptions(argc, argv);
    
    pMatrixVector* stats = readMatrixList(a.stat_file);
    assert(stats->size >= 2);

     
    Matrix *probs = stats->array[0];
    Matrix *fired = stats->array[1];
    Matrix *out = createMatrix(fired->nrow, (fired->ncol - fired->ncol % (int)a.dur)/a.dur);
    
    calcRun(fired, probs, out, a.dur, a.jobs);

    pMatrixVector *out_list = TEMPLATE(createVector,pMatrix)();
    TEMPLATE(insertVector,pMatrix)(out_list, out);
    saveMatrixList(a.output_file, out_list);
     
    TEMPLATE(deleteVector,pMatrix)(out_list);
    TEMPLATE(deleteVector,pMatrix)(stats);
    return(0);
}    


