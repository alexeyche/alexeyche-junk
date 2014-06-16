
#include "io.h"



pMatrixVector* readMatrixList(const char *filename) {
    FILE *f = fopen(filename, "rb");    
    if(f == NULL) {
        printf("Error in opening file %s\n", filename);
        return(NULL);
    }
    pMatrixVector *mlist = TEMPLATE(createVector,pMatrix)();
    
    while(1) {    
        unsigned int i = 0, j = 0;
        
        if(fread(&i, sizeof(unsigned int), 1, f) == 0) break;
        if(fread(&j, sizeof(unsigned int), 1, f) == 0) {
            printf("Error while reading second dimension in matrix list file %s\n", filename);
            break;
        }
        
        char type[100];
        size_t it = 0;
        while(1) {
            if(fread(&type[it], sizeof(char), 1, f) == 0) { 
                printf("Error while reading type in matrix list file %s\n", filename); 
                break;
            }
            if(type[it] == '\0') break;
            it++;
        }
        
        if(strcmp(type, "double") == 0) {
          Matrix *m = createMatrix(i, j); 
          if(fread(m->vals, sizeof(double), i*j, f) != i*j) {
              printf("Error while reading matrix in matrix list file %s\n", filename);
              break;
          }
          TEMPLATE(insertVector,pMatrix)(mlist, m);
        } else {
            printf("Found errors in input file %s\n", filename);        
            break;
        }
    }
    fclose(f);

    return(mlist);
}


void saveMatrixList(const char *filename, pMatrixVector *mv) {
    FILE *f = fopen(filename, "wb");    
    if(f == NULL) {
        printf("Error in opening file %s\n", filename);
        return;
    }
    size_t fi=strlen(filename)-1;
    while( (fi >= 0) ) {
        if( filename[fi] == '.' ) break;
        fi--;
    }
    if(fi == 0) { 
        printf("Need .bin format for matrix file\n");
        return;
    }
    const char *postfix = ".idx";
    char *idx_fname = (char*)malloc(fi+strlen(postfix)+1);
    strncpy(idx_fname, filename, fi);
    for(size_t ci=0; ci<strlen(postfix); ci++) {
        idx_fname[fi+ci] = postfix[ci];
    }
    idx_fname[fi+strlen(postfix)] = '\0';

    FILE *f_idx = fopen(idx_fname, "wb");    

    int null_pos = 0;
    fwrite(&null_pos, sizeof(int), 1, f_idx);
    const char *type_name = "double";
    for(size_t mi=0; mi < mv->size; mi++) {
        Matrix *m = mv->array[mi];        
        fwrite(&m->nrow, sizeof(unsigned int), 1, f);
        fwrite(&m->ncol, sizeof(unsigned int), 1, f);   
        fwrite(type_name, sizeof(char), strlen(type_name)+1, f);
        fwrite(m->vals, sizeof(double), m->nrow*m->ncol, f);
        
        int pos = ftell(f);
        fwrite(&pos, sizeof(int), 1, f_idx);
    }
    fclose(f);
    fclose(f_idx);
    free(idx_fname);
}
