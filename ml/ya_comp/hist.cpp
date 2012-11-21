

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
        min_out[j] = 0;
    }
    for(int i=0; i<nrow; i++) {
        for(int j=0; j<ncol; j++) {
            if( min_out[j] > x[i*ncol+j] ) {
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
        steps[j] = ((double)abs(max_w[j])+(double)abs(min_w[j]))/precision;
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

