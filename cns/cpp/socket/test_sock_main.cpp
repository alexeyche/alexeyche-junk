
#include <stdio.h>
#include "sim_client.c"

int main(int argc, char **argv) {
    int n = 10000;
    float msg[n];
    for(int i=0; i<n; i++) {
        msg[i]=0.001;
    }
    int len = sizeof(msg);
    char head[50] = "Var_name:1:10000@";
    printf("sizeof(msg): %i\n", len);
    init_socket();
    send_message_f(head, 50, msg, len);
}
