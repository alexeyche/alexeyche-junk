#include <cmath>

#include "pw_for_feat.cpp"

double* max(double *x, int nrow, int ncol) {
    double *max_out = (double*) malloc(ncol *sizeof(double));
    for(int j=0; j<ncol; j++) {
        max_out[j] = 0;
    }
    for(int i=0; i<nrow; i++) {
        for(int j=0; j<ncol; j++) {
            if( max_out[j] < x[i*ncol+j] ) {
               max_out[j] = x[i*ncol+j];
            }
        }
    }
    return max_out;
}

double* min(double *x, int nrow, int ncol) {
    double *min_out = (double*) malloc(ncol *sizeof(double));
    for(int j=0; j<ncol; j++) {
        min_out[j] = -1;
    }
    for(int i=0; i<nrow; i++) {
        for(int j=0; j<ncol; j++) {
            if( min_out[j] == -1) {
                min_out[j] = x[i*ncol+j];
            } else if( min_out[j] > x[i*ncol+j] ) {
               min_out[j] = x[i*ncol+j];
            }
        }
    }
    return min_out;
}

double* hist(double *x, int nrow, int ncol, int precision = 10000) {
    double *h = (double*) malloc(ncol * (precision+2) * sizeof(double));
    for(int i =0; i<precision; i++) {
        for(int j=0; j<ncol; j++) {
            h[i*ncol +j] = 0;
        }
    }
    double *steps = (double*) malloc(ncol * sizeof(double));
    double *max_w = max(x,nrow,ncol);
    double *min_w = min(x,nrow,ncol);
    for(int j=0; j<ncol; j++) {
        steps[j] = ((double)fabs(max_w[j])+(double)fabs(min_w[j]))/precision;
    }
    for(int i=0; i<nrow; i++) {
        for(int j=0; j<ncol; j++) {
            double val = x[i*ncol +j];
            int cell = abs(val/steps[j]);    
            if(cell>=precision) {
                cell = precision-1;
            }
            h[cell*ncol + j] += (double)1/nrow;
        }
    }   
    for(int j=0; j<ncol; j++) {
        h[precision*ncol+j] = min_w[j];
        h[(precision+1)*ncol+j] = max_w[j];
    }    
    return h;
}

void hist_func(char *in_filename, char *out_filename, int precision) {
    int nrow = count_rows(in_filename);
    int ncol = count_cols(in_filename, '\t');
    double *x = read_csv_file(in_filename,'\t',nrow,ncol);
    double *h = hist(x, nrow, ncol, precision);
    write_csv_file(h, out_filename,'\t',precision+2,ncol);    
    free(x);
    free(h);
}


double* hist_pw(double *x_all, int nrow, int ncol, double min,double max, int prec, double h) {
    double step = std::abs(std::abs(max) - std::abs(min))/prec;
    double *p_ret = new double[prec * ncol];
    double p = min;
    for(int j=0; j<prec; j++) {
        p += step;
        double *x = new double[ncol];
        for(int i=0; i<ncol; i++) {
            x[i] = p; 
        }
        for(int i=0; i<ncol; i++) {
            p_ret[j*ncol + i] = parzen_window(x, x_all, nrow, ncol, i, h);        
        }
        delete x;
    }
    return p_ret;
}

void hist_pw_func(char *in_filename, char *out_filename, int precision) {
    int nrow = count_rows(in_filename);
    int ncol = count_cols(in_filename, '\t');
    double *x = read_csv_file(in_filename,'\t',nrow,ncol);
    double *h = hist_pw(x, nrow, ncol, 0, 1, precision, 0.05);
    write_csv_file(h, out_filename,'\t',precision,ncol);    
    free(x);
    free(h);
}

