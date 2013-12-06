#include <stdio.h>

int koiwin(int);

int main(int argc, char **argv) {
    FILE* fps;
    FILE* fpt;
    int c;

    if(argc < 3) {
        fprintf(stderr, "Usage:");
        return(-7);
    }

    if((fps = fopen(argv[1], "r")) == NULL) {
        perror(argv[1]);
        return(2);
    }
    if((fpt = fopen(argv[2], "w")) == NULL) {
        perror(argv[2]);
        return(2);
    }
    while((c = fgetc(fps)) != EOF) {
        c = koiwin(c);
        fputc(c, fpt);
    }
    fclose(fps);
    fclose(fpt);

    return(0);
}

int koiwin(int c) {
    static char sub[32] = { 30, 0, 1, 22, 4, 5, 20, 3, 21, 8, 9, 10, 11, 12, 13, 14, 15, 31, 16, 17, 18, 19, 6, 2, 28, 27, 7, 24, 29, 25, 23, 26 };
    static unsigned char tab[64];
    static int i = 0;

    while(i<32) {
        tab[i+32] = sub[i] + 0300;
        tab[i] = sub[i] + 0340;
        i++;
        printf("%d\n", i);
    }    
    if(c < 0300) return(c);
    return(tab[c-0300]);
}    

