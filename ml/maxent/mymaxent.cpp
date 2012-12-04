#include <string.h>
#include <list>
#include <cstdio>
#include "maxent.h"
#include <stack>
#include "../count_lines.cpp"

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

void add_feat_to_model(ME_Model & model, double *feat, char *label, int nfeat = 7)
{
    ME_Sample samp(label);
    for(int i=0; i<nfeat; i++) {
        char *mark = (char*) malloc(10 * sizeof(char));
        sprintf(mark,"feat%d",i);
        samp.add_feature(mark,feat[i]);
    }
    model.add_training_sample(samp);
}

int process_file(char *filename, ME_Model & model, char *label, int ncol, int countLines = 0) {
    int buffer = 300;
    char *buf = (char*) malloc( buffer * sizeof(char));
    FILE *fp;
    int count = 0;
    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
        while ( fgets( buf, buffer, fp ) != NULL ) {
            double *spl = split(buf, '\t', ncol);
            for(unsigned char i=0; i<ncol; i++) {
                add_feat_to_model(model, spl, label, ncol);
            }
            if(count%100000 == 0) {
                printf("%d processed\n", count);
            }
            free(spl);
            count++;
            if(count>countLines) {
                break;
            }
        }                
        fclose( fp );
    }
    else
    {
        fprintf( stderr, "Error opening file %s\n", filename );
        free(buf);
        return 1;
    }
    free(buf);
}

char* basename(char *filename) {
    int n = strlen(filename);
    n--; // index
    char ch = filename[n];
    std::stack<char> symbols;
    while(ch != '/') {
        symbols.push(ch);
        n--;
        ch = filename[n];            
    }
    int n_s = symbols.size();
    char *out = new char[n_s+1];
    for(int i=0; i<n_s; i++) {
        out[i] = symbols.top();
        symbols.pop();
    }
    out[n_s] = '\0';
    return out;
}

int main(int argc, char *argv[]) {
    ME_Model model;
        
    char *train = argv[1];
    char *test = argv[2];
    char *name = basename(train);
    int nrow_train = count_rows(train);
    int nrow_test = count_rows(test);
    int ncol = count_cols(train, '\t');
    // for train data, bad label:
    char *bad_label = "bad";
    char *good_label = "good";
    process_file(train, model, bad_label, ncol, nrow_train);
    process_file(test, model, good_label, ncol, nrow_test);
    printf("Start training %s. nrow train: %d, nrow test: %d\n", name, nrow_train, nrow_test);
    model.train();
    char *model_name = new char[200];
    sprintf(model_name, "model_%s",name);    
    model.save_to_file(model_name);
    return 0;
}    
