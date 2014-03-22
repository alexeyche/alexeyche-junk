#ifndef ARG_OPT_H
#define ARG_OPT_H

#include <stdlib.h>
#include <string.h>


extern char *strdup(const char *s);

typedef struct {
    int jobs;
    const char *const_filename;
} ArgOptions;

void usage(void) {
    printf("Usage: \n");
    printf("\t-c - constants ini filename\n");
    printf("\t-j - number of parallel jobs\n");
    exit(8);
}

ArgOptions parseOptions(int argc, char **argv) {
    ArgOptions args;
    args.jobs = 1;
    if(argc == 1) usage();
    while ((argc > 1) && (argv[1][0] == '-')) {
        switch (argv[1][1]) {
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
    return(args);
}

#endif    
