#include <string.h>
#include <list>
#include <cstdio>
#include "maxent.h"

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

//char* label_sample(double *feat, int nfeat=7) {
//        char *str = (char*) malloc( 20 *sizeof(char));
//        strcpy(str,"");
//        if(feat[0]>4) {
//            strcat(str,"f1");
//        }
//        if(feat[1]>300){
//            strcat(str,"f2");
//        }
//        if(feat[2]>0.0005){
//            strcat(str,"f3");
//        }
//        if(feat[3]>0){
//            strcat(str,"f4");
//        }
//        if(feat[4]>0){
//            strcat(str,"f5");
//        }
//        if(feat[5]>0){
//            strcat(str,"f6");
//        }
//        if(feat[6]>0){
//            strcat(str,"f7");
//        }
//        return str;
//}

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

int process_file(char *filename, ME_Model & model, char *label, int countLines) {
    int ncols = 7;
    int buffer = 300;
    char *buf = (char*) malloc( buffer * sizeof(char));
    FILE *fp;
    int count = 0;
    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
        while ( fgets( buf, buffer, fp ) != NULL ) {
            double *spl = split(buf, '\t', ncols);
            for(unsigned char i=0; i<ncols; i++) {
                add_feat_to_model(model, spl, label);
            }
            if(count%100000 == 0) {
                printf("%d processed\n", count);
            }
            count++;
            free(spl);
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


int main(int argc, char *argv[]) {
    ME_Model model;
    int test_size = 4546558;
    char *train = argv[1];
    char *test = argv[2];
    // for train data, bad label:
    char *bad_label = "bad";
    char *good_label = "good";
    process_file(train, model, bad_label, test_size/6);
    process_file(test, model, good_label, test_size/10);
    printf("Start training\n");
    model.train();
    model.save_to_file("model");
    return 0;
}    
