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

template <typename T>
void send_message(T *msg, int len) {
    int c_len=len;
    int k;
    while(c_len > 0) {
        k = send(sock_desc, msg, c_len, 0);      
        if (k == -1)
        {
            printf("cannot write to server!\n");
            break;
        }
        printf("send k: %i\n", k);
        msg += k;
        c_len -= k;
    }   
}



void send_message_d(char *head, double *msg, int head_len, int msg_len) {
    printf("sending head: %i\n", head_len);
    send_message<char>(head, head_len);
    printf("sending msg: %i\n", msg_len);
    send_message<double>(msg, msg_len);
    close(sock_desc);  
}

