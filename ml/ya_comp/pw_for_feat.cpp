
#include <cmath>

double K(double x) {
    if(std::abs(x) > 1) {
        return 0;
    }
    return 0.75 *(1-x*x);
}


double parzen_window(double *x, double* x_all, int nrow, int ncol, int only_n=-1, double h = 0.1) {
    int n = ncol;
    int m = nrow;
    double Ksum = 0;
    for(int i=0; i<m; i++) {
        double Kprod = 1;
        for(int j=0; j<n; j++) {
            if((only_n != -1) && (only_n != j)) {
                continue;
            }    
            Kprod = Kprod * K(std::abs(x_all[i*ncol+j]-x[j])/h)/h;
            if(Kprod < 0) {
                printf("alert\n");
            }
        }
        Ksum += Kprod;
    }
    return Ksum/m;
}




