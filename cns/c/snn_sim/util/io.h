#ifndef IO_H_
#define IO_H_

#include <stdio.h>
#include <unistd.h>

#include <util/matrix.h>
#include <string.h>
#include <sys/stat.h>

typedef struct FileStream {
    FILE *fd;
    const char *fname;
} FileStream;

pMatrixVector* readMatrixList(FileStream *f, int num_to_read);
void saveMatrixList(FileStream *f, pMatrixVector *mv);

pMatrixVector* readMatrixListFromFile(const char *fname);
void saveMatrixListToFile(const char *fname, pMatrixVector *mv);

FileStream* createOutputFileStream(const char *fname);
FileStream* createInputFileStream(const char *fname);
void deleteFileStream(FileStream *fs);
char* getIdxName(const char *fname);
void checkIdxFnameOfModel(const char *fname);

#endif
