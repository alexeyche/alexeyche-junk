
#include <stdio.h>
#include "sim_server.c"

int main(int argc, char **argv) {
    args_s args;
    MessageCont *mc = MessageContCreate();
    args.mc = mc;
    args.port = 7778;
    printf("Creating thread\n");
    pthread_t t;
    pthread_create( &t, NULL, get_message, (void*)&args);
    sleep(100);
    pthread_join(t, NULL);
}
