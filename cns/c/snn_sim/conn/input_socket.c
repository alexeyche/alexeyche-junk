
#define BUFF_SIZE 4096


void listenForInput(int p) {
    int sock, listener;
    struct sockaddr_in addr;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("socket");
        exit(1);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(p);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Can't bind input port");
        exit(2);
    }
    listen(listener, 1);
    while(1)
    {
        sock = accept(listener, NULL, NULL);
        if(sock < 0)
        {
            perror("Can't accept input port");
            exit(3);
        }
        char *pageContentBuffer = (char*) malloc(BUFF_SIZE * sizeof(char));
        int amntRecvd;
        int i=0;
        while ((amntRecvd = recv(proxySocketFD, pageContentBuffer + i, BUFF_SIZE, 0)) > 0) {
            i += amntRecvd;
            pageContentBuffer = realloc(pageContentBuffer, i + BUFF_SIZE);
        }
        char *message_head = strdup(pageContentBuffer);
        if(strcmp(message_head, "MInputSpikes" ) == 0) {
            int head_len = strlen(message_head);
            MInputSpikes *m = deserialize_MInputSpikes(pageContentBuffer+head_len, amntRecvd-head_len);
        }
        
        free(message_head);
        free(pageContentBuffer);
        close(sock);
    }
}
