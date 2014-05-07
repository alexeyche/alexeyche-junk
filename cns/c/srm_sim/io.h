#ifndef IO_H_
#define IO_H_

#include <stdio.h>
#include <util/matrix.h>
#include <string.h>

pMatrixVector* readMatrixList(const char *filename);
void saveMatrixList(const char *filename, pMatrixVector *mv);

#endif
