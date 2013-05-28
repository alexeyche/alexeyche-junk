
#include <stdio.h>
#include "sim_client.c"

int main(int argc, char **argv) {
    float msg[] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.1, 1001.1, 1 };
    int len = sizeof(msg);
    printf("sizeof(msg): %i\n", len);
    init_socket();
    send_message_f(msg, len);
}
