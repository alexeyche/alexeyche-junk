
#include "postproc.h"

pMatrixVector *calcHists(SpikesList *spikes, doubleVector *timeline, double ts_dur, double ksize) {
    pMatrixVector *hists = TEMPLATE(createVector,pMatrix)();
    double t;
    size_t i;

    size_t spike_indices[spikes->size];
    for(size_t ni=0; ni < spikes->size; ni++) {
        spike_indices[ni] = 0;
    }
    double spike_per_cell = 1.0/ksize;

    for(t=0, i=0; fabs(timeline->size * ts_dur-t) > 0.00001; t+=ts_dur, i++) {
        size_t ncols = ceil(ts_dur / ksize);
//        printf("%zu\n", ncols);
        Matrix *hist = createZeroMatrix(spikes->size, ncols);
        for(size_t ni=0; ni < spikes->size; ni++) {
            if(spike_indices[ni] < spikes->list[ni]->size) {
                double sp_t = spikes->list[ni]->array[ spike_indices[ni] ];
                while((sp_t >= t)&&(sp_t < t+ts_dur)) {
                    size_t hist_index = floor((sp_t -t)/ksize);
//                    printf("(%f-%f)/%f == hist_index %zu \n", sp_t, t, ksize, hist_index);
                    if(hist_index >= ncols) {
                        printf("%zu >= %zu\n", hist_index, ncols);
                        exit(1);
                    }
                    double v = getMatrixElement(hist, ni, hist_index);
                    setMatrixElement(hist, ni, hist_index, v + spike_per_cell);
                    spike_indices[ni] += 1;
                    if(spike_indices[ni] >= spikes->list[ni]->size) break;
                    sp_t = spikes->list[ni]->array[ spike_indices[ni] ];
                }
            }
        }
        TEMPLATE(insertVector,pMatrix)(hists, hist);
    }
    return(hists);
}

double euclDistance(Matrix *m1, Matrix *m2) {
    assert((m1->nrow == m2->nrow) && (m1->ncol == m2->ncol));
    double d = 0;
    for(size_t i=0; i<m1->nrow; i++) {
        for(size_t j=0; j<m1->ncol; j++) {
            d += (getMatrixElement(m1, i, j) - getMatrixElement(m2, i, j))*(getMatrixElement(m1, i, j) - getMatrixElement(m2, i, j));
        }            
    }
    return(sqrt(d));
}


indVector* firstNNClassify(Matrix *K, indVector *train_labels_ind) {
    assert(K->ncol == train_labels_ind->size);

    indVector *predictedClasses = TEMPLATE(createVector,ind)();
    for(size_t testi=0; testi < K->nrow; testi++) {
        double min_dist = DBL_MAX;
        size_t predicted_class = 0;
        bool class_was_predicted = false;
        for(size_t traini=0; traini < K->ncol; traini++) {
            double dist = getMatrixElement(K, testi, traini);
            if(dist < min_dist) {
                predicted_class = train_labels_ind->array[traini];
                class_was_predicted = true;
                min_dist = dist;
            }
        }
        assert(class_was_predicted);
//        printf("%zu %zu %f\n", testi, predicted_class, min_dist);

        TEMPLATE(insertVector,ind)(predictedClasses, predicted_class);
    }
    return(predictedClasses);
}

void* kernelMatrixCalcJob(void *args) {
    KernelMatrixCalcWorker *kw = (KernelMatrixCalcWorker*) args;
    int neuron_per_thread = (kw->right->size + kw->nthreads - 1) / kw->nthreads;
    int first = min(  kw->thread_id    * neuron_per_thread, kw->right->size );
    int last  = min( (kw->thread_id+1) * neuron_per_thread, kw->right->size );
//    printf("%zu %d %d\n", kw->thread_id, first, last);
    for(size_t ri=0; ri<kw->left->size; ri++) {
        for(size_t ci=first; ci<last; ci++) {
            double eucl_dist = euclDistance(kw->left->array[ri], kw->right->array[ci]);
            setMatrixElement(kw->K, ri, ci, eucl_dist);
        }
    }
    pthread_barrier_wait( &barrier );
    return(NULL);

}

double judge_log(double v) {
    if(v==0.0) return(0.0);
    return(log(v));        
}

Matrix* calcConfMatrix(intVector *test, intVector *pred, intVector *uniq_classes) {
    Matrix *confM = createZeroMatrix(uniq_classes->size, uniq_classes->size);
    for(size_t el_i=0; el_i < test->size; el_i++) {
//        printf("%d %d\n",  test->array[ el_i ]-1, pred->array[ el_i ]-1);
        incMatrixElement(confM, test->array[ el_i ]-1, pred->array[ el_i ]-1, 1);
//        printf("%f", getMatrixElement(confM, test->array[ el_i ]-1, pred->array[ el_i ]-1));
    }
    return confM;
}

double calcNMI(Matrix *confM) {
    double log_cl = log(confM->nrow);
    
    double sum_of_col[confM->nrow];
    for(size_t i=0; i<confM->nrow; i++) { sum_of_col[i]=0.0; }
    double sum_of_row[confM->ncol];
    for(size_t i=0; i<confM->ncol; i++) { sum_of_row[i]=0.0; }

    printf("%zu %zu\n", confM->nrow, confM->ncol);
    double whole_sum = 0;
    for(size_t i=0; i<confM->nrow; i++) {
        for(size_t j=0; j<confM->ncol; j++) {
            double el = getMatrixElement(confM, i, j);
            sum_of_row[j] += el;
            sum_of_col[i] += el;
            whole_sum += el;
        }
    }

    double NMI = 0;
    for(size_t i=0; i<confM->nrow; i++) {
        for(size_t j=0; j<confM->ncol; j++) {
            double Nij = getMatrixElement(confM, i, j);
            double v = judge_log(Nij)/log_cl;        
            v -= judge_log(sum_of_row[j])/log_cl;
            v -= judge_log(sum_of_col[i])/log_cl;
            v += judge_log(whole_sum)/log_cl;
            NMI += v * Nij;
        }
    }
    NMI = NMI/whole_sum;
    return NMI;
}

ClassificationStat getClassificationStat(pMatrixVector *train, indVector *train_labels_ind, pMatrixVector *test, indVector *test_labels_ind, intVector *uniq_classes, int  jobs) {
    Matrix *K = createMatrix(test->size, train->size);
    
    pthread_t *threads = (pthread_t *) malloc( jobs * sizeof( pthread_t ) );
    KernelMatrixCalcWorker *workers = (KernelMatrixCalcWorker*) malloc( jobs * sizeof(KernelMatrixCalcWorker) );
    for(size_t wi=0; wi < jobs; wi++) {
        workers[wi].thread_id = wi;
        workers[wi].left = test;
        workers[wi].right = train;
        workers[wi].K = K;
        workers[wi].nthreads = jobs;
    }
    pthread_attr_t attr;
    P( pthread_attr_init( &attr ) );
    P( pthread_barrier_init( &barrier, NULL, jobs ) );
    for( int i = 1; i < jobs; i++ )  {
        P( pthread_create( &threads[i], &attr, kernelMatrixCalcJob,  &workers[i]) );
    }
    kernelMatrixCalcJob(&workers[0]);

    free(workers);
    free(threads);
    
    indVector *predictedClasses = firstNNClassify(K, train_labels_ind);
    assert(predictedClasses->size == test_labels_ind->size);
    
    ClassificationStat s;

    s.confM = createZeroMatrix(uniq_classes->size, uniq_classes->size);
    double num_errors = 0.0;
    for(size_t el_i=0; el_i < test_labels_ind->size; el_i++) {
        incMatrixElement(s.confM, test_labels_ind->array[ el_i ], predictedClasses->array[ el_i ], 1);
        if( predictedClasses->array[ el_i ] != test_labels_ind->array[ el_i ] ) {
            num_errors += 1;
        }
    }

    double log_cl = log(uniq_classes->size);
    
    double sum_of_col[s.confM->nrow];
    double sum_of_row[s.confM->ncol];
    double whole_sum = 0;
    for(size_t i=0; i<s.confM->nrow; i++) {
        if(i == 0) sum_of_col[i] = 0;
        for(size_t j=0; j<s.confM->ncol; j++) {
            if((i == 0)&&(j==0)) sum_of_row[j] = 0;
            double el = getMatrixElement(s.confM, i, j);
            sum_of_row[j] += el;
            sum_of_col[i] += el;
            whole_sum += el;
        }
    }
//    for(size_t i=0; i<s.confM->nrow; i++) {
//        printf("%1.1f ",  sum_of_col[i]);
//    }
//    printf("\n");
//    for(size_t j=0; j<s.confM->ncol; j++) {
//        printf("%1.1f ",  sum_of_row[j]);
//    }
//    printf("\n");
    double NMI = 0;
    for(size_t i=0; i<s.confM->nrow; i++) {
        for(size_t j=0; j<s.confM->ncol; j++) {
            double Nij = getMatrixElement(s.confM, i, j);
            double v = judge_log(Nij)/log_cl;        
            v -= judge_log(sum_of_row[j])/log_cl;
            v -= judge_log(sum_of_col[i])/log_cl;
            v += judge_log(whole_sum)/log_cl;
            NMI += v * Nij;
        }
    }
    s.NMI = NMI/whole_sum;
    s.rate = num_errors/test_labels_ind->size;
    return(s);
}

void writeMatrixListToSVMStruct(pMatrixVector *v, doubleVector *cl, const char *output_file) {
    assert(v->size == cl->size);
    FileStream *fs = createOutputFileStream(output_file);
    for(size_t i=0; i<v->size; i++) {
        Matrix *m = v->array[i];
        writeLineSVMStruct(m->vals, m->ncol*m->nrow, (int)cl->array[i], fs);
    }
}


void writeLineSVMStruct(double *vals, size_t size, int class, FileStream *fs) {
    fprintf(fs->fd, "%d ", class);
    for(size_t i=0; i<size; i++) {
        fprintf(fs->fd, "%zu:%5.8f ", i+1, vals[i]);
    }
    fprintf(fs->fd, "\n");
}


