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
    const char *model_file_save;
    const char *model_file_load;
    const char *output_spikes_file;
    const char *input_spikes_file;
    int seed;
    char learn;
} ArgOptions;

void printArgs(const ArgOptions *a) {
    printf("a->jobs = %d\n", a->jobs);
    printf("a->const_filename = %s\n", a->const_filename);
    printf("a->stat_file = %s\n", a->stat_file);
    printf("a->model_file = %s\n", a->model_file);
    printf("a->model_file_save = %s\n", a->model_file_save);
    printf("a->model_file_load = %s\n", a->model_file_load);
    printf("a->output_spikes_file = %s\n", a->output_spikes_file);
    printf("a->input_spikes_file = %s\n", a->input_spikes_file);
    printf("a->seed = %d\n", a->seed);
}

void usage(void) {
    printf("Usage: \n");
    printf("\t-c - constants ini filename\n");
    printf("\t-j - number of parallel jobs\n");
    printf("\t-s - stat file\n");
    printf("\t-m(s|l) - model file for save and load or save or load\n");
    printf("\t-i - file for input spikes\n");
    printf("\t-o - file for output spikes\n");
    printf("\t-l - yes/no to learn\n");
    printf("\t-seed - integer seed\n");
    printf("\t-? - print this message\n");
    exit(8);
}

ArgOptions parseOptions(int argc, char **argv) {
    ArgOptions args;
    args.jobs = 1;
    args.const_filename = NULL;
    args.stat_file = NULL;
    args.model_file = NULL;
    args.model_file_save = NULL;
    args.model_file_load = NULL;
    args.output_spikes_file = NULL;
    args.input_spikes_file = NULL;
    args.learn = -1;
    args.seed = time(NULL);
    if(argc == 1) usage();
    while ((argc > 1) && (argv[1][0] == '-')) {
        if(strcmp(argv[1], "-m") == 0) {                
            if(argc == 2) { 
                printf("No options for -m\n");
                usage();
            }
            args.model_file = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-i") == 0) {                
            if(argc == 2) { 
                printf("No options for -i\n");
                usage();
            }
            args.input_spikes_file = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-o") == 0) {                
            if(argc == 2) { 
                printf("No options for -o\n");
                usage();
            }
            args.output_spikes_file = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-ms") == 0) {                
            if(argc == 2) { 
                printf("No options for -ms\n");
                usage();
            }
            args.model_file_save = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-ml") == 0) {                
            if(argc == 2) { 
                printf("No options for -ml\n");
                usage();
            }
            args.model_file_load = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-c") == 0) {                
            if(argc == 2) { 
                printf("No options for -c\n");
                usage();
            }
            args.const_filename = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-j") == 0) {                
            if(argc == 2) { 
                printf("No options for -j\n");
                usage();
            }
            args.jobs = atoi(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-s") == 0) {                
             if(argc == 2) { 
                printf("No options for -s\n");
                usage();
            }
            args.stat_file = strdup(argv[2]);
            ++argv; --argc;
        } else 
        if(strcmp(argv[1], "-seed") == 0) {                
            if(argc == 2) { 
                printf("No options for -s\n");
                usage();
            }
            args.seed = atoi(argv[2]);
            ++argv; --argc;
        } else 
        if(strcmp(argv[1], "-l") == 0) {                
            args.learn = 1;
            if(argc == 2) { 
                args.learn = 1;
            } else
            if(strcmp(argv[2], "yes") == 0) {
                args.learn = 1;
            } else 
            if(strcmp(argv[2], "no") == 0) {
                args.learn = 0;
            }
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-?") == 0) {                
            usage();   
        } else {            
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
    if((args.model_file) && ( (args.model_file_save) || (args.model_file_load))) {
        printf("Doubling in options. Choose model for save or for load or for both of them\n");
        usage();
    }

    return(args);
}

#endif    
