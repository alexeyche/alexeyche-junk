#include "maxent.h"

#include <string.h>
#include <list>
#include <cstdio>

using namespace std;

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

int main(int argc, char *argv[]) {
    ME_Model model;
    model.load_from_file("model");
    int ncols = 7;
    char *filename = argv[1];
    int buffer = 300;
    char *buf = (char*) malloc( buffer * sizeof(char));
    FILE *fp;
    int count = 0;
    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
        while ( fgets( buf, buffer, fp ) != NULL ) {
            ME_Sample s;
            double *spl = split(buf, '\t', ncols);
            for(unsigned char i=0; i<ncols; i++) {
                char *mark = (char*) malloc(10 * sizeof(char));
                sprintf(mark,"feat%d",i);               
                s.add_feature(mark,spl[i]);
            }
            vector<double> vp = model.classify(s);
            for (int i = 0; i < model.num_classes(); i++) {
                cout << count << "\t" << vp[i] << "\t" <<  model.get_class_label(i) << "\t";
            }
            count++;
            cout << endl;            
            free(spl);
        }                
        fclose( fp );
    }
    else
    {
        fprintf( stderr, "Error opening file %s\n", filename );
        free(buf);
        return 1;
    }
}



