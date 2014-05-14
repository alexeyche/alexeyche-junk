

#include <core.h>
#include <args/calc_opts.h>

#include <util/io.h>
#include <util/matrix.h>
#include <math.h>

#include <calc.h>

int main(int argc, char **argv) {
	ArgOptionsCalc a = parseCalcOptions(argc, argv);
    
    pMatrixVector* stats = readMatrixList(a.stat_file);
    assert(stats->size == 3);

     
    Matrix *probs = stats->array[0];
    Matrix *fired = stats->array[1];
    Matrix **out = (Matrix**)malloc(sizeof(Matrix*) * fired->nrow);

    Matrix *classes = stats->array[2];
    assert(classes->ncol == (fired->ncol - fired->ncol % (int)a.dur)/a.dur);    
    intVector *uniq_classes = TEMPLATE(createVector,int)();
    indVector *classes_indices = TEMPLATE(createVector,ind)(); 

    for(size_t i=0; i<classes->ncol; i++) {
        int index = -1;
        for(size_t ci=0; ci < uniq_classes->size; ci++) {
            if(uniq_classes->array[ci] == (int)getMatrixElement(classes, 0, i)) {
                index = ci;
            }
        }
        if((index<0) || (uniq_classes->size == 0)) {
            TEMPLATE(insertVector,int)(uniq_classes, (int)getMatrixElement(classes, 0, i));
        }
        if(index<0) index = uniq_classes->size-1;
        TEMPLATE(insertVector,ind)(classes_indices, index);
    }
    //for(size_t ci=0; ci<classes_indices->size; ci++) {
    //    printf("%zu %d %f\n", classes_indices->array[ci], uniq_classes->array[classes_indices->array[ci]], getMatrixElement(classes, 0, ci));
    //}
    
    calcRun(fired, probs, out, uniq_classes, classes_indices, a.dur, a.jobs);
    
    pMatrixVector *out_list = TEMPLATE(createVector,pMatrix)();
    for(size_t ni=0; ni < fired->nrow; ni++) {
        TEMPLATE(insertVector,pMatrix)(out_list, out[ni]);
    }
    saveMatrixList(a.output_file, out_list);
     
    TEMPLATE(deleteVector,pMatrix)(out_list);
    
    free(out);
    TEMPLATE(deleteVector,pMatrix)(stats);
    return(0);
}    


