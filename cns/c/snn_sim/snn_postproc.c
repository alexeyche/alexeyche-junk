
#include <core.h>

#include <args/postproc_opts.h>
#include <util/io.h>
#include <util/spikes_list.h>
#include <util/matrix.h>

#include <postproc.h>

SpikesList* cutSpikesList(SpikesList *sl, int number_to_cut_from_head) {
    int new_size = sl->size - number_to_cut_from_head;
    assert(new_size>0);
    SpikesList *new_sl = createSpikesList(new_size);
    for(size_t i = number_to_cut_from_head, j=0; i<sl->size; i++, j++) {
        TEMPLATE(deleteVector,double)(new_sl->list[j]);
        new_sl->list[j] = TEMPLATE(copyVector,double)(sl->list[i]);
    }
    deleteSpikesList(sl);
    return(new_sl);
}

int main(int argc, char **argv) {
    ArgOptionsPostProc a = parsePostProcOptions(argc, argv);
    if(a.calc_predict_stat_table) {
        intVector *uniq_classes_calc_stat_left = TEMPLATE(createVector,int)();
        intVector *uniq_classes_calc_stat_right = TEMPLATE(createVector,int)();
        intVector *classes_calc_stat_left = TEMPLATE(createVector,int)();
        intVector *classes_calc_stat_right = TEMPLATE(createVector,int)();
        FILE *s; 
        if((s = fopen(a.calc_predict_stat_table, "r")) == NULL) {
            perror("Error open");
        }
        char *line;
        size_t len = 0;
        ssize_t read;
        while((read = getline(&line, &len, s)) != -1) {
            int left, right;
            sscanf (line, "%d %d", &left, &right);
            
            int index_left = -1;
            for(size_t ci=0; ci < uniq_classes_calc_stat_left->size; ci++) {
                if(uniq_classes_calc_stat_left->array[ci] == left) {
                    index_left = ci;
                }
            }                
            if(index_left<0) {
                TEMPLATE(insertVector,int)(uniq_classes_calc_stat_left, left);
            }

            int index_right = -1;
            for(size_t ci=0; ci < uniq_classes_calc_stat_right->size; ci++) {
                if(uniq_classes_calc_stat_right->array[ci] == right) {
                    index_right = ci;
                }
            }                
            if(index_right<0) {
                TEMPLATE(insertVector,int)(uniq_classes_calc_stat_right, right);
            }
            TEMPLATE(insertVector,int)(classes_calc_stat_right, right);
            TEMPLATE(insertVector,int)(classes_calc_stat_left, left);
        }
        if(line) {
            free(line);
        }
        fclose(s);  /* close the file prior to exiting the routine */
        

        assert(uniq_classes_calc_stat_right->size == uniq_classes_calc_stat_left->size);
        Matrix *confM = calcConfMatrix(classes_calc_stat_left, classes_calc_stat_right, uniq_classes_calc_stat_left);
        for(size_t i=0; i<confM->nrow; i++) {
            for(size_t j=0; j<confM->nrow; j++) {
                printf("%3.1f ", getMatrixElement(confM, i, j));
            }
            printf("\n");
        }
        printf("NMI = %f\n", calcNMI(confM));
        deleteMatrix(confM);
        return(0);
    }
   
//train data    
    pMatrixVector *input_train_struct = readMatrixListFromFile(a.input_train_spikes);
    
    assert(input_train_struct->size == 3);
    
    Matrix *spikes_train_m = input_train_struct->array[0];
    SpikesList *spikes_train = spikesMatrixToSpikesList(spikes_train_m);
    if(a.ignore_first_neurons>0) {
        spikes_train = cutSpikesList(spikes_train, a.ignore_first_neurons);
    }
    Matrix *timeline_train_m = input_train_struct->array[1];
    Matrix *classes_train_m = input_train_struct->array[2];
    doubleVector *classes_train = TEMPLATE(copyFromArray,double)(classes_train_m->vals, classes_train_m->nrow*classes_train_m->ncol);
    doubleVector *timeline_train = TEMPLATE(copyFromArray,double)(timeline_train_m->vals, timeline_train_m->nrow*timeline_train_m->ncol);
    double dur = timeline_train->array[1] - timeline_train->array[0];
//test data    
    pMatrixVector *input_test_struct = readMatrixListFromFile(a.input_test_spikes);
    
    assert(input_test_struct->size == 3);
    
    Matrix *spikes_test_m = input_test_struct->array[0];
    SpikesList *spikes_test = spikesMatrixToSpikesList(spikes_test_m);
    if(a.ignore_first_neurons>0) {
        spikes_test = cutSpikesList(spikes_test, a.ignore_first_neurons);
    }
    Matrix *timeline_test_m = input_test_struct->array[1];
    Matrix *classes_test_m = input_test_struct->array[2];
    doubleVector *classes_test = TEMPLATE(copyFromArray,double)(classes_test_m->vals, classes_test_m->nrow*classes_test_m->ncol);
    doubleVector *timeline_test = TEMPLATE(copyFromArray,double)(timeline_test_m->vals, timeline_test_m->nrow*timeline_test_m->ncol);    
// classes    
    intVector *uniq_classes = TEMPLATE(createVector,int)();
    indVector *classes_indices_train = TEMPLATE(createVector,ind)(); 
    indVector *classes_indices_test = TEMPLATE(createVector,ind)(); 

    for(size_t i=0; i<classes_train->size; i++) {
        int index = -1;
        for(size_t ci=0; ci < uniq_classes->size; ci++) {
            if(uniq_classes->array[ci] == (int)classes_train->array[i]) {
                index = ci;
            }
        }
        if((index<0) || (uniq_classes->size == 0)) {
            TEMPLATE(insertVector,int)(uniq_classes, (int)classes_train->array[i]);
        }
        if(index<0) index = uniq_classes->size-1;
        TEMPLATE(insertVector,ind)(classes_indices_train, index);
    }
    for(size_t i=0; i<classes_test->size; i++) {
        int index = -1;
        for(size_t ci=0; ci < uniq_classes->size; ci++) {
            if(uniq_classes->array[ci] == (int)classes_test->array[i]) {
                index = ci;
            }
        }
        assert(index >= 0);
        TEMPLATE(insertVector,ind)(classes_indices_test, index);
    }
    pMatrixVector *stat = TEMPLATE(createVector,pMatrix)();
// kernel calc    
    double max_NMI = DBL_MIN;
    for(size_t ki=0; ki < a.kernel_values->size; ki++) {    
        double ksize = a.kernel_values->array[ki];
//        printf("Calc for ksize == %f ... \n", ksize);
    // train hists
        pMatrixVector *hists_train = calcHists(spikes_train, timeline_train, dur, ksize);
//        printf("%zu %zu\n", hists_train->size, classes_train->size);
        assert(hists_train->size == classes_train->size);
    // test hists
        pMatrixVector *hists_test = calcHists(spikes_test, timeline_test, dur, ksize);
        assert(hists_test->size == classes_test->size);
        
    //=================
//        TEMPLATE(deleteVector,pMatrix)(hists_train);
//        TEMPLATE(deleteVector,pMatrix)(hists_test);
//    
//        hists_test = readMatrixListFromFile("/home/alexeyche/prog/sim/ts/synthetic_control/synthetic_control_TEST_1000.bin");
//        hists_train = readMatrixListFromFile("/home/alexeyche/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_1000.bin");
    //================    
        if(a.classify_one_nn) {
            ClassificationStat s = getClassificationStat(hists_train, classes_indices_train, hists_test, classes_indices_test, uniq_classes, a.jobs);
            if(a.output_file) {
                TEMPLATE(insertVector,pMatrix)(stat, copyMatrix(s.confM));
                Matrix *r = createMatrix(1,1);
                setMatrixElement(r, 0, 0, s.rate);
                TEMPLATE(insertVector,pMatrix)(stat, r);

                Matrix *i = createMatrix(1,1);
                setMatrixElement(i, 0, 0, s.NMI);
                TEMPLATE(insertVector,pMatrix)(stat, i);
            }
            if(max_NMI < s.NMI) {
                max_NMI = s.NMI;
            }
            deleteMatrix(s.confM);
        } else {
            char output_file_kernel[256];
            snprintf(output_file_kernel, sizeof output_file_kernel, "%s.train.k%3.1f", a.output_file, ksize);
            if(!a.svm_out) {
                char buf[256];
                snprintf(buf, sizeof buf, "%s.bin", output_file_kernel);
                saveMatrixListToFile(buf, hists_train);        
            } else {
                char buf[256];
                snprintf(buf, sizeof buf, "%s.dat", output_file_kernel);
                writeMatrixListToSVMStruct(hists_train, classes_train, buf);        
            }
            char output_file_kernel_test[256];
            snprintf(output_file_kernel_test, sizeof output_file_kernel_test, "%s.test.k%3.1f", a.output_file, ksize);
            if(!a.svm_out) {
                char buf[256];
                snprintf(buf, sizeof buf, "%s.bin", output_file_kernel_test);
                saveMatrixListToFile(buf, hists_test);        
            } else {
                char buf[256];
                snprintf(buf, sizeof buf, "%s.dat", output_file_kernel_test);
                writeMatrixListToSVMStruct(hists_test, classes_test, buf);        
            }
            
        }
//        printf("rate: %f\n", s.rate); 
//        for(size_t i=0; i < s.confM->nrow; i++) {
//            for(size_t j=0; j < s.confM->ncol; j++) {
//                printf("%1.1f ", getMatrixElement(s.confM, i, j));
//            }
//            printf("\n");
//        }
//        printf("NMI: %f\n", s.NMI);
        TEMPLATE(deleteVector,pMatrix)(hists_train);
        TEMPLATE(deleteVector,pMatrix)(hists_test);
    }

    if(a.classify_one_nn) {
        if(a.output_file) {
            saveMatrixListToFile(a.output_file, stat);
        }
        printf("%f\n", max_NMI);
    }
    TEMPLATE(deleteVector,pMatrix)(stat);    
    TEMPLATE(deleteVector,double)(timeline_train);
    TEMPLATE(deleteVector,double)(timeline_test);

    TEMPLATE(deleteVector,double)(a.kernel_values);
    TEMPLATE(deleteVector,pMatrix)(input_train_struct);
    TEMPLATE(deleteVector,pMatrix)(input_test_struct);
    deleteSpikesList(spikes_train);
    deleteSpikesList(spikes_test);
    return(0);
}
 
