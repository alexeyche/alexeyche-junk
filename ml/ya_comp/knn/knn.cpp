
#include <math.h>
#include "../read_csv.cpp"
#include "../count_lines.cpp"

#define SQUARE(val) val*val

double euclidean_dist(double *x1, double *x2, int n) {
    double sq_dist = 0;
    for(int i=0; i<n; i++) {
        sq_dist = SQUARE(x1[i]-x2[i]);
    }
}

double* knn(double *x, int m, int n) {
    for(int i=0; i<m; i++) {
        for(int j=0; j<n; j++) {
                    
        }
    }
}


int main(int argc, char *argv[]) {
    char *input_f;
    if(argc>1) {
        input_f = argv[1];
    } else {
        printf("Need input file\n");
        return 1;
    }
    int nrow = count_rows(input_f);
    int ncol = count_cols(input_f, '\t');
    double *x = read_csv_file(input_f,'\t',nrow,ncol);
    double *y = knn(x, nrow, ncol);
    return 0;
}


