#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _LARGEFILE_SOURCE 1
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS   64

int count_cols(char *filename, char delim) {
    unsigned int col_count = 0;
    FILE *fp;
    unsigned int buffer = 1000;
    char *buf = (char*) malloc(buffer * sizeof(char));
    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
        char *first_row;
        if (fgets( buf, buffer, fp ) == NULL) {
            fclose( fp );
            free(buf);
            return 0;
        }
        fclose( fp );
        unsigned int i = 0;
        while(buf[i] != '\n') {
            if(buf[i] == '\t') {
                col_count++;
            }
            i++;
        }
        col_count++;

    }
    free(buf);
    return col_count;
}


int count_rows(char *filename) {
    FILE *fp;
    unsigned int buffer = 256;
    char *buf = (char*) malloc(buffer * sizeof(char));
    int count = 0;
    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
        while ( fgets( buf, buffer, fp ) != NULL ) {
            count++;
        }                
        fclose( fp );
    }
    free(buf);
    return count;
}
