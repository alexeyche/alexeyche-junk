#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFELEM 100
#define FILENAME "parse_out"
#define NROWS 7856733
#define NROWS_T 738996
#define NFEAT 4
#define LENGTH(x) (sizeof(x)/sizeof(*(x)))


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
        if(line[i] == delim) {
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


double K(double x) {
    if(abs(x) > 1) {
        return 0;
    }
    return 0.75 *(1-x*x);
}


double parzen_window(double *x, double* x_all, double h) {
    int n = NFEAT;
    int m = NROWS;
    double Ksum = 0;
    for(int i=0; i<m; i++) {
        double Kprod = 1;
        for(int j=0; j<n; j++) {
            Kprod = Kprod * K(abs(x_all[i*NFEAT+j]-x[j])/h)/h;
        }
        Ksum += Kprod;
    }
    return Ksum/m;
}



double *read_csv_file(const char *filename, char delim, int nrows, int ncols, int buffer = 100) {
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
        fprintf( stderr, "Error opening file %s\n", FILENAME );
        free(buf);
        free(x);
        return NULL;
    }
    free(buf);
    return x;   
}


int main( void )
{
    const char *train = "parse_out";
    const char *test = "parse_out.test";
    FILE *outfile;
    outfile = fopen("answer_l","w");
    double *x = read_csv_file(train,'\t',NROWS,NFEAT);
    double *x_test = read_csv_file(test,'\t',NROWS_T,NFEAT);

    for(int i=0; i<NROWS_T; i++) {
        double *x_test_row = splice(x_test,i*NFEAT, i*NFEAT+NFEAT);
        double p = parzen_window(x_test_row, x, 0.5);
        fprintf(outfile, "%d\t%.10f\n",i,p); 
        free(x_test_row);
    }

    fclose(outfile);
    free(x);
    free(x_test);
    return EXIT_SUCCESS;
}


