
#include <core/util/util.h>

double normal_distr_var = -1;

double getUnif() {
    return (double)rand()/(double)RAND_MAX;
}


double getExp(double rate) {
    double u = getUnif();
    return(-log(u)/rate);
}

double getNorm() {
    if(normal_distr_var<0) {
        double U = getUnif(); 
        double V = getUnif(); 
        normal_distr_var = sqrt(-2*log(U)) * cos(2*PI*V);
        return(sqrt(-2*log(U)) * sin(2*PI*V));
    } else {
        double ret = normal_distr_var;
        normal_distr_var=-1;
        return(ret);        
    }
}

double getLogNorm(double logmean, double logsd) {
    return( exp(logmean + logsd*getNorm()) );
}

void swapInd(size_t *l, size_t *r) {
    size_t temp = *r;
    *r = *l;
    *l = temp;
}

void shuffleIndVector(indVector *v) {
    for(int i=(v->size-1); i>0; i--) {
        int j = rand() % (i+1);
        swapInd(&v->array[i], &v->array[j]);
    }
}




