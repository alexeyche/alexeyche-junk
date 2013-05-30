
#include <stdio.h>
#include "sim_client.c"

#define HEAD_LEN 100

int main(int argc, char **argv) {
    int n = 100;
    double msg[n];
    msg[0] = -1;
    for(int i=1; i<n; i++) {
        msg[i]=0.001;
    }
    int len = sizeof(msg);
    char head[HEAD_LEN] = "Var_name:1:100@";
    init_socket(7778);
    send_message_d(head, msg, HEAD_LEN, len);
}
