

#include <sys/socket.h>  
#include <netinet/in.h>  
#include <stdio.h>  
#include <string.h> 
#include <stdlib.h>  
#include <arpa/inet.h> 
#include <unistd.h> 
int sock_desc;

void run_server(int port = 7777) {
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1)
    {
        printf("cannot create socket!\n");
        return;
    }
    struct sockaddr_in server;  
    memset(&server, 0, sizeof(server));  
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;  
    server.sin_port = htons(port);  
    if (bind(sock_desc, (struct sockaddr*)&server, sizeof(server)) != 0)
    {
        printf("cannot bind socket!\n");
        close(sock_desc);  
        return;
    }

    if (listen(sock_desc, 20) != 0)
    {
        printf("cannot listen on socket!\n");
        close(sock_desc);  
        return;
    } 
    struct sockaddr_in client;  
    memset(&client, 0, sizeof(client));  
    socklen_t len = sizeof(client); 
    int temp_sock_desc = accept(sock_desc, (struct sockaddr*)&client, &len);  
    if (temp_sock_desc == -1)
    {
        printf("cannot accept client!\n");
        close(sock_desc);  
        return;
    }   
    
    float buf[1000];  
    int k;  
    printf("In main loop\n");
    while(1) 
    {      
        k = recv(temp_sock_desc, buf, 1000, 0);      
        if (k == -1)
        {
            printf("\ncannot read from client!\n");
            break;
        }
        if(k > 0) {
            printf("k:%i\n", k);
            for(int i=0; i<k/4; i++) { printf("%f,", buf[i]); }
            printf("\n");
        }
        if(k == 0) {
            break;
        }
    }

    close(temp_sock_desc);  
    close(sock_desc);  

    printf("server disconnected\n");
    return;  
}

