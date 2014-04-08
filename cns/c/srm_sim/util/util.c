
#include <util/util.h>

double getUnif() {
    return (double)rand()/(double)RAND_MAX;
}


double getExp(double rate) {
    double u = getUnif();
    return(-log(u)/rate);
}
