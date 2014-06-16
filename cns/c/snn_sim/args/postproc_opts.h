#ifndef POSTPROC_OPTS_H
#define POSTPROC_OPTS_H


#include <stdlib.h>
#include <string.h>

#include <util/util_vector.h>
#include <util/matrix.h>


typedef struct {
    const char *input_train_spikes;
    const char *input_test_spikes;
    doubleVector *kernel_values;
    const char *output_file;
    int jobs;
    int ignore_first_neurons;
} ArgOptionsPostProc;

void usagePostProc(void) {
    printf("Usage: \n");
    printf("\t-i - input file with train spikes\n");
    printf("\t-t - input file with test spikes\n");
    printf("\t-o - output file with stat\n");
    printf("\t-k - kernel range for postprocess (start:delta:end)\n");
    printf("\t-j - jobs\n");
    printf("\t--ignore-first-neurons - number neurons to ingore\n");
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
    args.input_train_spikes = NULL;
    args.input_test_spikes = NULL;
    args.output_file = NULL;
    args.jobs = 1;
    args.ignore_first_neurons = 0;
    if(argc == 1) usagePostProc();
    while ((argc > 1) && (argv[1][0] == '-')) {
        if(strcmp(argv[1], "--ignore-first-neurons") == 0) {                
             if(argc == 2) { 
                 printf("No options for --ignore-first-neurons\n");
                 usagePostProc();
             }
             args.ignore_first_neurons = atoi(argv[2]);
             ++argv; --argc;
        } else 
        if(strcmp(argv[1], "-j") == 0) {                
             if(argc == 2) { 
                 printf("No options for -o\n");
                 usagePostProc();
             }
             args.jobs = atoi(argv[2]);
             ++argv; --argc;
        } else             
        if(strcmp(argv[1], "-t") == 0) {                
             if(argc == 2) { 
                 printf("No options for -o\n");
                 usagePostProc();
             }
             args.input_test_spikes = strdup(argv[2]);
             ++argv; --argc;
        } else             
        if(strcmp(argv[1], "-o") == 0) {                
             if(argc == 2) { 
                 printf("No options for -o\n");
                 usagePostProc();
             }
             args.output_file = strdup(argv[2]);
             ++argv; --argc;
        } else             
        if(strcmp(argv[1], "-k") == 0) {                
             if(argc == 2) { 
                 printf("No options for -k\n");
                 usagePostProc();
             }
             fillKernelRange(kernel_range, argv[2]);
             ++argv; --argc;
        } else             
        if(strcmp(argv[1], "-i") == 0) {
            if(argc == 2) { 
                printf("No options for -i\n");
                usagePostProc();
            }
            args.input_train_spikes = strdup(argv[2]);
            ++argv; --argc;
        } else {            
            printf("Wrong Argument: %s\n", argv[1]);
            usagePostProc();
        }
        ++argv; --argc;
    }
    if(!args.input_train_spikes) {
        printf("Need input train spikes file\n");
        usagePostProc();
    }
    if(!args.input_test_spikes) {
        printf("Need input test spikes file\n");
        usagePostProc();
    }
    if(kernel_range->size != 3) {
        printf("Kernels range is inappropriate\n");
        usagePostProc();
    }
    if(args.jobs < 0) {
        printf("Jobs number is inappropriate\n");
        usagePostProc();
    }
    for(double k=kernel_range->array[0]; k <= kernel_range->array[2]; k+=kernel_range->array[1]) { 
        TEMPLATE(insertVector,double)(args.kernel_values, k);
    }
    return(args);
}


#endif
