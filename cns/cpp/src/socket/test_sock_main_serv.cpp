
#include <stdio.h>
#include "sim_server.c"

int main(int argc, char **argv) {
    printf("-1\n");
    pthread_t t;
    MessageCont *mc = run_server(7777, &t);
    printf("1\n");
    pthread_join(t, NULL);
    printf("3\n");
}
