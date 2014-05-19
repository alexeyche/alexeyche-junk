#ifndef POSTPROC_OPTS_H
#define POSTPROC_OPTS_H


#include <stdlib.h>
#include <string.h>

#include <util/util_vector.h>
#include <util/matrix.h>


typedef struct {
    const char *input_spikes;
    double dur;
    doubleVector *kernel_values;
} ArgOptionsPostProc;

void usagePostProc(void) {
    printf("Usage: \n");
    printf("\t-i - input file with spikes\n");
    printf("\t-d - duration of series\n");
    printf("\t-k - kernel range for postprocess (start:delta:end)\n");
    printf("\t-? - print this message\n");
    exit(8);
}

void fillKernelRange(doubleVector *v, const char *vals) {
    char *token;
    char *string = strdup(vals);
    while ((token = strsep(&string, ":")) != NULL) {
        TEMPLATE(insertVector,double)(v, atof(token));
    }
    free(string);
}

ArgOptionsPostProc parsePostProcOptions(int argc, char **argv) {
    ArgOptionsPostProc args;
    doubleVector *kernel_range = TEMPLATE(createVector,double)();

    args.kernel_values = TEMPLATE(createVector,double)();
    args.input_spikes = NULL;
    args.dur = 0;
    if(argc == 1) usagePostProc();
    while ((argc > 1) && (argv[1][0] == '-')) {
        if(strcmp(argv[1], "-k") == 0) {                
             if(argc == 2) { 
                 printf("No options for -k\n");
                 usagePostProc();
             }
             fillKernelRange(kernel_range, argv[2]);
             ++argv; --argc;
        } else {            
        if(strcmp(argv[1], "-d") == 0) {                
             if(argc == 2) { 
                 printf("No options for -d\n");
                 usagePostProc();
             }
             args.dur = atof(argv[2]);
             ++argv; --argc;
        } else {            
            if(strcmp(argv[1], "-i") == 0) {                
            if(argc == 2) { 
                printf("No options for -i\n");
                usagePostProc();
            }
            args.input_spikes = strdup(argv[2]);
            ++argv; --argc;
        } else {            
            printf("Wrong Argument: %s\n", argv[1]);
            usagePostProc();
        }
        ++argv; --argc;
    }
    if(!args.input_spikes) {
        printf("Need input spikes file\n");
        usagePostProc();
    }
    if(args.dur <= 0) {
        printf("Need normal value for duration of series (>0)\n");
        usagePostProc();
    }
    if(kernel_range->size != 3) {
        printf("Kernels range is inappropriate\n");
        usagePostProc();
    }
    for(double k=kernel_range->array[0]; k < kernel_range->array[2]; k+=kernel_range->array[1]) { 
        TEMPLATE(insertVector,double)(args.kernel_values, k);
    }
    return(args);
}


#endif
