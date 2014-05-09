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
    int input_port;
    int output_port;
    int seed;
    char learn;
} ArgOptionsSim;


void printArgs(const ArgOptionsSim *a) {
    printf("a->jobs = %d\n", a->jobs);
    printf("a->const_filename = %s\n", a->const_filename);
    printf("a->stat_file = %s\n", a->stat_file);
    printf("a->model_file = %s\n", a->model_file);
    printf("a->model_file_save = %s\n", a->model_file_save);
    printf("a->model_file_load = %s\n", a->model_file_load);
    printf("a->output_spikes_file = %s\n", a->output_spikes_file);
    printf("a->input_spikes_file = %s\n", a->input_spikes_file);
    printf("a->seed = %d\n", a->seed);
    printf("a->input_port = %d\n", a->input_port);
    printf("a->output_port = %d\n", a->output_port);
}

void usageSim(void) {
    printf("Usage: \n");
    printf("\t-c - constants ini filename\n");
    printf("\t-j - number of parallel jobs\n");
    printf("\t-s - stat file\n");
    printf("\t-m(s|l) - model file for save and load or save or load\n");
    printf("\t-i - file for input spikes\n");
    printf("\t-o - file for output spikes\n");
    printf("\t-l - yes/no to learn\n");
    printf("\t-seed - integer seed\n");
    printf("\t-ip - port for input spikes\n");
    printf("\t-op - port for output spikes\n");
    printf("\t-? - print this message\n");
    exit(8);
}



ArgOptionsSim parseSimOptions(int argc, char **argv) {
    ArgOptionsSim args;
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
    args.input_port = -1;
    args.output_port = -1;
    if(argc == 1) usageSim();
    while ((argc > 1) && (argv[1][0] == '-')) {
        if(strcmp(argv[1], "-m") == 0) {                
            if(argc == 2) { 
                printf("No options for -m\n");
                usageSim();
            }
            args.model_file = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-i") == 0) {                
            if(argc == 2) { 
                printf("No options for -i\n");
                usageSim();
            }
            args.input_spikes_file = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-ip") == 0) {                
            if(argc == 2) { 
                printf("No options for -ip\n");
                usageSim();
            }
            args.input_port = atoi(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-op") == 0) {                
            if(argc == 2) { 
                printf("No options for -op\n");
                usageSim();
            }
            args.output_port = atoi(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-o") == 0) {                
            if(argc == 2) { 
                printf("No options for -o\n");
                usageSim();
            }
            args.output_spikes_file = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-ms") == 0) {                
            if(argc == 2) { 
                printf("No options for -ms\n");
                usageSim();
            }
            args.model_file_save = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-ml") == 0) {                
            if(argc == 2) { 
                printf("No options for -ml\n");
                usageSim();
            }
            args.model_file_load = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-c") == 0) {                
            if(argc == 2) { 
                printf("No options for -c\n");
                usageSim();
            }
            args.const_filename = strdup(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-j") == 0) {                
            if(argc == 2) { 
                printf("No options for -j\n");
                usageSim();
            }
            args.jobs = atoi(argv[2]);
            ++argv; --argc;
        } else
        if(strcmp(argv[1], "-s") == 0) {                
             if(argc == 2) { 
                printf("No options for -s\n");
                usageSim();
            }
            args.stat_file = strdup(argv[2]);
            ++argv; --argc;
        } else 
        if(strcmp(argv[1], "-seed") == 0) {                
            if(argc == 2) { 
                printf("No options for -s\n");
                usageSim();
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
            usageSim();   
        } else {            
            printf("Wrong Argument: %s\n", argv[1]);
            usageSim();
        }

        ++argv; --argc;
    }
    if(args.jobs == 0) {
        printf("Jobs number is inappropriate\n");
        usageSim();
    }
    if(args.const_filename == NULL) {
        printf("Need const file name\n");
        usageSim();
    }
    if((args.model_file) && ( (args.model_file_save) || (args.model_file_load))) {
        printf("Doubling in options. Choose model for save or for load or for both of them\n");
        usageSim();
    }
    if((args.input_spikes_file == NULL) && (args.input_port < 0)) {
        printf("Need something on input\n");
        usageSim();
    }
    if((args.output_spikes_file == NULL) && (args.output_port < 0)) {
        printf("Need something on output\n");
        usageSim();
    }
    return(args);
}

#endif    
