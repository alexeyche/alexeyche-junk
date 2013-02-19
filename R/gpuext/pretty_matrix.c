#include <stdio.h>
#include <string.h>
#include "R.h"
#include "Rdefines.h"
#include "R_ext/Rdynload.h"


SEXP pretty_matrix(SEXP s_matrix) {
    int *matrix, width, height, row, col, i;
    SEXP retval;
    char tmp[10];
    
    if (isMatrix(s_matrix) && isInteger(s_matrix)) {
        matrix = INTEGER(s_matrix);
        width  = INTEGER(GET_DIM(s_matrix))[1];
        height = INTEGER(GET_DIM(s_matrix))[0];
    }
    else {
        printf("invalid matrix.\n");
        return R_NilValue;
    }
    
    // allocate memory for pretty matrix strings
    char *text[4];
    for (i = 0; i < 4; i++)
        text[i] = (char *)malloc(sizeof(char) * 5 * width + 2);
    
    // print out matrix prettily
    char header[5 * width + 2];
    header[0] = '+';
    header[1] = 0;
    for (i = 0; i < width; i++)
        strcat(header, "----+");
    printf("%s\n", header);
    
    for (row = 0; row < height; row++) {
        text[0][0] = text[1][0] = text[2][0] = text[3][0] = 0;
        
        for (col = 0; col < width; col++) {
            sprintf(tmp, "| %2d ", matrix[col*height + row]);
            
            strcat(text[0], "|    ");
            strcat(text[1], tmp);
            strcat(text[2], "|    ");
            strcat(text[3], "+----");
        }
        strcat(text[0], "|");
        strcat(text[1], "|");
        strcat(text[2], "|");
        strcat(text[3], "+");
        
        printf("%s\n%s\n%s\n%s\n", text[0], text[1], text[2], text[3]);
    }

    // free memory
    for (i = 0; i < 4; i++)
        free(text[i]);
    
    PROTECT(retval = NEW_NUMERIC(1));
    DOUBLE_DATA(retval)[0] = (4 * height + 1) ;
    UNPROTECT(1);

    return retval;
}

R_CallMethodDef callMethods[] =
{
    {"pretty_matrix", (DL_FUNC)&pretty_matrix, 1},
    {NULL,NULL, 0}
};

void R_init_pretty_matrix(DllInfo *dll)
{
    R_registerRoutines(dll,NULL,callMethods,NULL,NULL);
}
