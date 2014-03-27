
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
        }
    }
    fclose(f);

    return(mlist);
}
