
#include "io.h"

FileStream* createOutputFileStream(const char *fname) {
    FileStream *fs = (FileStream*) malloc(sizeof(FileStream));
    fs->fd = fopen(fname, "wb");
    fs->fname = strdup(fname); 
    return(fs);
}

FileStream* createInputFileStream(const char *fname) {
    FileStream *fs = (FileStream*) malloc(sizeof(FileStream));
    fs->fd = fopen(fname, "rb");
    fs->fname = strdup(fname); 
    return(fs);
}

void deleteFileStream(FileStream *fs) {
    fclose(fs->fd);
    free(fs);
}

pMatrixVector* readMatrixListFromFile(const char *fname) {
    FileStream *fs = createInputFileStream(fname);
    pMatrixVector *data = readMatrixList(fs, -1);
    deleteFileStream(fs);
    return(data);
}

pMatrixVector* readMatrixList(FileStream *f, int num_to_read) {
    if(f == NULL) {
        printf("Error in opening file %s\n", f->fname);
        return(NULL);
    }
    pMatrixVector *mlist = TEMPLATE(createVector,pMatrix)();
    int matrix_read = 0;

    while(1) {    
        if((matrix_read >= num_to_read)&&(num_to_read>0)) {
            break;
        }
        unsigned int i = 0, j = 0;
        
        if(fread(&i, sizeof(unsigned int), 1, f->fd) == 0) break;
        if(fread(&j, sizeof(unsigned int), 1, f->fd) == 0) {
            printf("Error while reading second dimension in matrix list file %s\n", f->fname);
            break;
        }
        
        char type[100];
        size_t it = 0;
        while(1) {
            if(fread(&type[it], sizeof(char), 1, f->fd) == 0) { 
                printf("Error while reading type in matrix list file %s\n", f->fname); 
                break;
            }
            if(type[it] == '\0') break;
            it++;
        }
        
        if(strcmp(type, "double") == 0) {
            Matrix *m = createMatrix(i, j); 
            if(fread(m->vals, sizeof(double), i*j, f->fd) != i*j) {
                printf("Error while reading matrix in matrix list file %s\n", f->fname);
                break;
            }
            TEMPLATE(insertVector,pMatrix)(mlist, m);
            matrix_read += 1; 
        } else {
            printf("Found errors in input file %s\n", f->fname);        
            break;
        }
    }
    return(mlist);
}


void saveMatrixList(FileStream *f, pMatrixVector *mv) {
    if(f == NULL) {
        printf("Error in opening file %s\n", f->fname);
        return;
    }
    size_t fi=strlen(f->fname)-1;
    while( (fi >= 0) ) {
        if( f->fname[fi] == '.' ) break;
        fi--;
    }
    if(fi == 0) { 
        printf("Need .bin format for matrix file\n");
        return;
    }
    const char *postfix = ".idx";
    char *idx_fname = (char*)malloc(fi+strlen(postfix)+1);
    strncpy(idx_fname, f->fname, fi);
    for(size_t ci=0; ci<strlen(postfix); ci++) {
        idx_fname[fi+ci] = postfix[ci];
    }
    idx_fname[fi+strlen(postfix)] = '\0';

    FILE *f_idx = fopen(idx_fname, "ab");    

    int null_pos = 0;
    fwrite(&null_pos, sizeof(int), 1, f_idx);
    const char *type_name = "double";
    for(size_t mi=0; mi < mv->size; mi++) {
        Matrix *m = mv->array[mi];        
        fwrite(&m->nrow, sizeof(unsigned int), 1, f->fd);
        fwrite(&m->ncol, sizeof(unsigned int), 1, f->fd);   
        fwrite(type_name, sizeof(char), strlen(type_name)+1, f->fd);
        fwrite(m->vals, sizeof(double), m->nrow*m->ncol, f->fd);
        
        int pos = ftell(f);
        fwrite(&pos, sizeof(int), 1, f_idx);
    }
    fclose(f_idx);
    free(idx_fname);
}

void saveMatrixListToFile(const char *fname, pMatrixVector *mv) {
    FileStream *fs = createOutputFileStream(fname);
    saveMatrixList(fs, mv);
    deleteFileStream(fs);
}
