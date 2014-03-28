#ifndef ARG_OPT_H
#define ARG_OPT_H

#include <stdlib.h>
#include <string.h>


extern char *strdup(const char *s);

typedef struct {
    int jobs;
    const char *const_filename;
    const char *stat_file;
    const char *model_file;
} ArgOptions;

void usage(void) {
    printf("Usage: \n");
    printf("\t-c - constants ini filename\n");
    printf("\t-j - number of parallel jobs\n");
    printf("\t-s - stat file\n");
    printf("\t-m - model file\n");
    exit(8);
}

ArgOptions parseOptions(int argc, char **argv) {
    ArgOptions args;
    args.jobs = 1;
    args.const_filename = NULL;
    args.stat_file = NULL;
    args.model_file = NULL;
    if(argc == 1) usage();
    while ((argc > 1) && (argv[1][0] == '-')) {
        switch (argv[1][1]) {
            case 'm':
                if(argc == 2) { 
                    printf("No options for -c\n");
                    usage();
                }
                args.model_file = strdup(argv[2]);
                ++argv; --argc;
                break;

            case 'c':
                if(argc == 2) { 
                    printf("No options for -c\n");
                    usage();
                }
                args.const_filename = strdup(argv[2]);
                ++argv; --argc;
                break;

            case 'j':
                if(argc == 2) { 
                    printf("No options for -j\n");
                    usage();
                }
                args.jobs = atoi(argv[2]);
                ++argv; --argc;
                break;
            case 's':
                if(argc == 2) { 
                    printf("No options for -c\n");
                    usage();
                }
                args.stat_file = strdup(argv[2]);
                ++argv; --argc;
                break;



            default:
                printf("Wrong Argument: %s\n", argv[1]);
                usage();
        }

        ++argv; --argc;
    }
    if(args.jobs == 0) {
        printf("Jobs number is inappropriate\n");
        usage();
    }
    if(args.const_filename == NULL) {
        printf("Need const file name\n");
        usage();
    }
    return(args);
}

#endif    
