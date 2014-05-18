#ifndef ARG_OPT_PREP_H
#define ARG_OPT_PREP_H

#include <stdlib.h>
#include <string.h>


//extern char *strdup(const char *s);


typedef struct {
    int jobs;
    const char *const_filename;
    const char *input_file;
    const char *input_labels_file;
    const char *output_file;
    const char *stat_file;
} ArgOptionsPrep;


void usagePrep(void) {
    printf("Usage: \n");
    printf("\t-c - constants ini filename\n");
    printf("\t-i - file for input ts\n");
    printf("\t-il - file for labels in input file\n");
    printf("\t-o - file for output spikes\n");
    printf("\t-s - file for statistics\n");
    printf("\t-? - print this message\n");
    exit(8);
}

ArgOptionsPrep parsePrepOptions(int argc, char **argv) {
    ArgOptionsPrep args;
    args.jobs = 1;
    args.const_filename = NULL;
    args.input_file = NULL;
    args.output_file = NULL;
    args.input_labels_file = NULL;
    args.stat_file = NULL;
    if(argc == 1) usagePrep();
    while ((argc > 1) && (argv[1][0] == '-')) {
        if(strcmp(argv[1], "-s") == 0) {                
            if(argc == 2) { 
                printf("No options for -s\n");
                usagePrep();
            }
            args.stat_file = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-o") == 0) {                
            if(argc == 2) { 
                printf("No options for -o\n");
                usagePrep();
            }
            args.output_file = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-il") == 0) {                
            if(argc == 2) { 
                printf("No options for -i\n");
                usagePrep();
            }
            args.input_labels_file = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-i") == 0) {                
            if(argc == 2) { 
                printf("No options for -i\n");
                usagePrep();
            }
            args.input_file = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-j") == 0) {                
            if(argc == 2) { 
                printf("No options for -j\n");
                usagePrep();
            }
            args.jobs = atoi(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-c") == 0) {                
            if(argc == 2) { 
                printf("No options for -c\n");
                usagePrep();
            }
            args.const_filename = strdup(argv[2]);
            ++argv; --argc;
        } else {            
            printf("Wrong Argument: %s\n", argv[1]);
            usagePrep();
        }

        ++argv; --argc;
    }
    if(args.jobs == 0) {
        printf("Jobs number is inappropriate\n");
        usagePrep();
    }
    if(args.const_filename == NULL) {
        printf("Need const file name\n");
        usagePrep();
    }
    if(args.input_labels_file  == NULL) {
        printf("Need input file name\n");
        usagePrep();
    }
    if(args.input_file  == NULL) {
        printf("Need input file name\n");
        usagePrep();
    }
    if(args.output_file  == NULL) {
        printf("Need output file name\n");
        usagePrep();
    }
    return(args);         
}



#endif
