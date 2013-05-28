#include <sys/socket.h>  
#include <netinet/in.h>  
#include <stdio.h>  
#include <string.h>  
#include <stdlib.h> 
#include <arpa/inet.h> 
#include <unistd.h> 

int sock_desc;

void init_socket(int port = 7777) {
    sock_desc = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_desc == -1)
    {
        printf("cannot create socket!\n");
        return;
    }

    struct sockaddr_in client;  
    memset(&client, 0, sizeof(client));  
    client.sin_family = AF_INET;  
    client.sin_addr.s_addr = inet_addr("127.0.0.1");  
    client.sin_port = htons(port);  
    if (connect(sock_desc, (struct sockaddr*)&client, sizeof(client)) != 0)
    {
        printf("cannot connect to server!\n");
        close(sock_desc);
        return;
    }
    printf("Socket connection established\n");
}


void send_message(char *msg) {
    int len = strlen(msg);
    int k = send(sock_desc, msg, len, 0);      
    if (k == -1)
    {
        printf("cannot write to server!\n");
    }
}

void send_message_f(float *msg, int len) {
    printf("sending %i\n", len);
    int c_len=len;
    int k;
    while(c_len > 0) {
        k = send(sock_desc, msg, c_len, 0);      
        if (k == -1)
        {
            printf("cannot write to server!\n");
        }
        printf("send k: %i\n", k);
        msg += k;
        c_len -= k;
    }
    //char c = '\n';
    //k = send(sock_desc, &c, 1, 0);      
    close(sock_desc);  
}

