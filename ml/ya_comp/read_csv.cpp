#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char* spliceChar(char *array, int start, int end) {
    if (end<=start) {
        return NULL;
    }
    int delta = end - start;
    char *splice_out = (char*) malloc(delta * sizeof(char));
    for(unsigned int i=0; i<delta;i++) {
        splice_out[i] = array[start+i];
    }
    return splice_out;
}

double* splice(double *array, int start, int end) {
    if (end<=start) {
        return NULL;
    }
    int delta = end - start;
    double *splice_out = (double*) malloc(delta * sizeof(double));
    for(unsigned int i=0; i<delta;i++) {
        splice_out[i] = array[start+i];
    }
    return splice_out;
}

double* split(char *line, char delim,char delCount) {
    double *ret = (double*) malloc(delCount * sizeof(double));
    int length = strlen(line);
    unsigned int ret_c = 0;
    unsigned int last_d = 0;
    for(unsigned int i=0; i<length; i++) {
        if ((line[i] == delim) || (line[i] == '\n')) {
            char *buff = spliceChar(line,last_d,i);
            if(buff == NULL) {
                break;
            }
            ret[ret_c] = atof(buff);
            free(buff);
           
            ret_c++;
            if(ret_c == delCount) {
                break;
            }
            last_d = i; 
        }
    }
    return ret;
}

double *read_csv_file(const char *filename, char delim, int nrows, int ncols, int buffer = 300) {
    double *x = (double*) malloc(nrows * ncols * sizeof(double));
    char *buf = (char*) malloc(buffer * sizeof(char));
    FILE *fp;
    char test_buf[ buffer ];
    int count=0;
    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
        while ( fgets( buf, buffer, fp ) != NULL ) {
            double *spl = split(buf, delim, ncols);
            for(unsigned char i=0; i<ncols; i++) {
                x[ count * ncols + i ] = spl[i];           
            }
            count++;
            free(spl);
        }                
        fclose( fp );
    }
    else
    {
        fprintf( stderr, "Error opening file %s\n", filename );
        free(buf);
        free(x);
        return NULL;
    }
    free(buf);
    return x;   
}

void write_csv_file(double *x, const char *filename, char delim, int nrow, int ncol) {
    FILE *fp = fopen(filename, "w");
    if(fp) {
        for(int i=0; i<nrow; i++) {
            for(int j=0; j<ncol; j++) {
                if(j<(ncol-1)) {
                    fprintf(fp, "%f%c",x[i*ncol+j],delim);
                } else {
                    fprintf(fp,"%f\n",x[i*ncol+j]);
                }                
            }
        }
        fclose(fp);
    }
}

