#ifndef CALC_OPTS_H
#define CALC_OPTS_H

#include <stdlib.h>
#include <string.h>


//extern char *strdup(const char *s);

typedef struct {
    int jobs;
    const char *stat_file;
} ArgOptionsCalc;

void usageCalc(void) {
    printf("Usage: \n");
    printf("\t-j - number of parallel jobs\n");
    printf("\t-s - file for stat file with probabilites and spikes\n");
    printf("\t-? - print this message\n");
    exit(8);
}


ArgOptionsCalc parseCalcOptions(int argc, char **argv) {
    ArgOptionsCalc args;
    args.jobs = 1;
    args.stat_file = NULL;
    if(argc == 1) usageCalc();
    while ((argc > 1) && (argv[1][0] == '-')) {
        if(strcmp(argv[1], "-s") == 0) {                
             if(argc == 2) { 
                printf("No options for -s\n");
                usageCalc();
            }
            args.stat_file = strdup(argv[2]);
            ++argv; --argc;
        } else 
        if(strcmp(argv[1], "-j") == 0) {                
            if(argc == 2) { 
                printf("No options for -j\n");
                usageCalc();
            }
            args.jobs = atoi(argv[2]);
            ++argv; --argc;
        } else {            
            printf("Wrong Argument: %s\n", argv[1]);
            usageCalc();
        }
        ++argv; --argc;
    }
    if(args.jobs == 0) {
        printf("Jobs number is inappropriate\n");
        usageCalc();
    }
    if(!args.stat_file) {
        printf("Need stat file with probabilites\n");
        usageCalc();
    }
    return(args);
}

#endif
