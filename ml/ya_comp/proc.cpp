
#include <string.h>
#include "normalize.cpp"
#include "hist.cpp"

void help() {
    printf("Available functions:\n");
    printf("\tcount_rows <filename>\n");
    printf("\tcount_cols <filename>\n");
}

int main(int argc, char *argv[]) {
    char *func_name;
    if(argc>1) {
        func_name = argv[1];
    } else {
        help();
        return 1;
    }
    if(strcmp(func_name,"count_rows") == 0) {
        char *filename;
        if(argc>2) {
            filename = argv[2];
        } else {
            help();
            return 1;
        }
        int count = count_rows(filename);
        printf("%d\n",count);
        return 0;
    } else if (strcmp(func_name,"count_cols") == 0) {
        char *filename;
        if(argc>2) {
            filename = argv[2];
        } else {
            help();
            return 1;
        }
        int count = count_cols(filename,'\t');
        printf("%d\n",count);
        return 0;
    } else if (strcmp(func_name,"mean") == 0) {
        char *filename;
        if(argc>2) {
            filename = argv[2];
        } else {
            help();
            return 1;
        }
        mean_func(filename);
        return 0;
    } else if (strcmp(func_name,"sd") == 0) {
        char *filename;
        if(argc>2) {
            filename = argv[2];
        } else {
            help();
            return 1;
        }
        sd_func(filename);
        return 0;
    } else if (strcmp(func_name,"norm") == 0) {
        char *in_filename;
        char *out_filename;
        if(argc>3) {
            in_filename = argv[2];
            out_filename = argv[3];
        } else {
            help();
            return 1;
        }
        normalize_func(in_filename,out_filename);
        return 0;
    } else if (strcmp(func_name,"hist") == 0) {
        char *in_filename;
        char *out_filename;
        int precision;
        if(argc>4) {
            in_filename = argv[2];
            out_filename = argv[3];
            precision = atoi(argv[4]);
        } else {
            help();
            return 1;
        }
        hist_func(in_filename,out_filename, precision);
        return 0;
    } else if (strcmp(func_name,"hist_pw") == 0) {
        char *in_filename;
        char *out_filename;
        int precision;
        if(argc>4) {
            in_filename = argv[2];
            out_filename = argv[3];
            precision = atoi(argv[4]);
        } else {
            help();
            return 1;
        }
        hist_pw_func(in_filename,out_filename, precision);
        return 0;
    } else {
        help();
        return 1;
    }
}

