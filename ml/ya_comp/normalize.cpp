#include <string.h>
#include <math.h>

#include "count_lines.cpp"
#include "read_csv.cpp"

double* mean(double *x, int nrow, int ncol) {
    double *means = (double*) malloc(ncol *sizeof(double));
    for(int j=0; j<ncol; j++) {
        means[j] = 0;
    }
    for(int i=0; i<nrow; i++) {
        for(int j=0; j<ncol; j++) {
            double val = x[i*ncol+j];
            means[j] = means[j] + val;
        }
    }
    for(int j=0; j<ncol; j++) {
        means[j] = means[j]/nrow;
    }
    return means;
}

double* sd(double *x, int nrow, int ncol, double *mean_x = NULL) {
    if(mean_x == NULL) {
        mean_x = mean(x, nrow, ncol);    
    } 
    double *sds = (double*) malloc(ncol *sizeof(double));    
    for(int j=0; j<ncol; j++) {
        sds[j] = 0;
    }
    for(int i=0; i<nrow; i++) {
        for(int j=0; j<ncol; j++) {
            double val = x[i*ncol+j];
            sds[j] += pow(val - mean_x[j], 2)/nrow;
        }
    }
    for(int j=0; j<ncol; j++) {
        sds[j] = sqrt(sds[j]);
    }
    return sds;
}


void mean_func(char *filename) {
    int nrow = count_rows(filename);
    int ncol = count_cols(filename, '\t');
    double *x = read_csv_file(filename,'\t',nrow,ncol);
    double *m = mean(x, nrow, ncol);
    for(int i=0; i<ncol; i++) {
        printf("%f\t",m[i]);
    }
    printf("\n");
}

void sd_func(char *filename) {
    int nrow = count_rows(filename);
    int ncol = count_cols(filename, '\t');
    double *x = read_csv_file(filename,'\t',nrow,ncol);
    double *d = sd(x, nrow, ncol);
    for(int i=0; i<ncol; i++) {
        printf("%f\t",d[i]);
    }
    printf("\n");
}

void normalize(double *x, int nrow, int ncol) {
    double *m = mean(x, nrow, ncol);
    double *s = sd(x, nrow, ncol, m);
    for(int i=0; i<nrow; i++) {
        for(int j=0; j<ncol; j++) {
            x[i*ncol + j] = (x[i*ncol + j] - m[j])/s[j];
        }
    }
}

void normalize_func(char *in_filename, char *out_filename) {
    int nrow = count_rows(in_filename);
    int ncol = count_cols(in_filename, '\t');
    double *x = read_csv_file(in_filename,'\t',nrow,ncol);
    normalize(x, nrow, ncol);
    write_csv_file(x, out_filename,'\t',nrow,ncol);    
}

