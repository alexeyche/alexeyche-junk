

#include <sys/socket.h>  
#include <netinet/in.h>  
#include <stdio.h>  
#include <string.h> 
#include <stdlib.h>  
#include <arpa/inet.h> 
#include <unistd.h> 
#include <pthread.h>

int sock_desc;

char* spliceChar(char *array, int start, int end) {
    if (end<=start) {
        return NULL;
    }
    int delta = end - start;
    char *splice_out = (char*)malloc(sizeof(char)*(delta+1));
    for(int i=0; i<delta;i++) {
        splice_out[i] = array[start+i];
    }
    splice_out[delta] = '\0';
    return splice_out;
}

//#define DEBUG

#define HEAD_LEN 100

typedef struct {
    char *var_name;
    int nrow;
    int ncol;
} HeadMessage;


HeadMessage read_head(int temp_sock_desc) {
    char buf[HEAD_LEN];
    int k = recv(temp_sock_desc, buf, HEAD_LEN, 0);     
    unsigned int last_i=0;
    char pos=0;
    char *var_name;
    int nrow;
    int ncol;
    for(unsigned int i=0; i<HEAD_LEN; i++) {
        if ((buf[i] == ':')&&(pos == 0)) {
            var_name = spliceChar(buf, last_i, i);                   
            last_i = i;
            pos++;
        } else 
        if ((buf[i] == ':')&&(pos == 1)) {
            char *nrow_c = spliceChar(buf, last_i+1, i+1);                   
            nrow=atoi(nrow_c);
            last_i = i;
            pos++;
            free(nrow_c);
        } else
        if ((buf[i] == '@')&&(pos == 2)) {
            char *ncol_c = spliceChar(buf, last_i+1, i+1);                   
            ncol=atoi(ncol_c);
            pos++;
            free(ncol_c);
            break;
        }
    }
    HeadMessage m; 
    m.var_name = var_name;
    m.nrow = nrow;
    m.ncol = ncol; 
    return m;
}

typedef struct {
    double *x;
    char *name;
    int nrow;
    int ncol;
} DoubleMessage;


typedef struct {
    DoubleMessage **cont;
    int num;
    int num_read;
}  MessageCont;

MessageCont *MessageContCreate() {
    MessageCont *mc = (MessageCont*) malloc(sizeof(MessageCont));
    mc->num = 0;
    mc->num_read = 0;
    mc->cont = NULL;
    return mc;
}
void MessageCont_add(MessageCont *mc, DoubleMessage *m) { 
    void *tmp = realloc(mc->cont, (mc->num+1) * sizeof(DoubleMessage*));
    if(!tmp) {
        printf("Can't allocate memory\n");        
        return;
    }
    mc->cont = (DoubleMessage**) tmp;
    mc->cont[mc->num] = m; 
    mc->num+=1; 
}
 
typedef struct {
    int port;
    MessageCont *mc;
} args_s;

//#define DEBUG

void* get_message(void *args_p) {
    #ifdef DEBUG
        printf("Starting thread\n");
    #endif 
    args_s *args = (args_s*) args_p;
    MessageCont *mes_cont = args->mc;
    int port = args->port;
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1)
    {
        printf("cannot create socket!\n");
        return NULL;
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
        return NULL;
    }

    if (listen(sock_desc, 20) != 0)
    {
        printf("cannot listen on socket!\n");
        close(sock_desc);  
        return NULL;
    } 
    struct sockaddr_in client;  
    memset(&client, 0, sizeof(client));  
    socklen_t len = sizeof(client); 
    #ifdef DEBUG
        printf("Go accepting\n");
    #endif 
    for(;;) {
        int temp_sock_desc = accept(sock_desc, (struct sockaddr*)&client, &len);  
        if (temp_sock_desc == -1)
        {
            printf("cannot accept client!\n");
            close(sock_desc);  
            return NULL;
        }   
        HeadMessage m = read_head(temp_sock_desc);
        #ifdef DEBUG
            printf("Got message: %s %d:%d\n", m.var_name, m.nrow, m.ncol);            
        #endif
        double *out = (double*) malloc(sizeof(double)*m.nrow*m.ncol);
        int buf_size=1000;
        if(m.nrow*m.ncol<buf_size) {
            buf_size = m.nrow*m.ncol;
        }
        int need_download = m.nrow*m.ncol;
        double buf[buf_size];
        int k;  
        int k_all = 0;
        while(1)
        {     
            k = recv(temp_sock_desc, buf, buf_size*sizeof(double), 0);      
            if (k == -1)
            {
                printf("\ncannot read from client!\n");
                break;
            }
            if(k > 0) {
                memcpy(out+k_all, buf, k);
                k_all += k/sizeof(double);
                #ifdef DEBUG                    
                    printf("--| read %d bytes (k_all: %d)\n", k,k_all);
                    printf("--| head of out: %f %f %f\n", out[0],out[1],out[2]);
                    printf("--| head of buf: %f %f %f\n", buf[0],buf[1],buf[2]);
                #endif
                need_download -= k/sizeof(double);
                if(need_download<=0) {
                    #ifdef DEBUG
                        printf("--| we've downloaded all arrray\n");
                    #endif
                    break;
                }
            }
            if(k == 0) {
                break;
            }
        }

        close(temp_sock_desc);  
        DoubleMessage *fm = (DoubleMessage*)malloc(sizeof(DoubleMessage));
        fm->x = out;

        fm->name = m.var_name;
        fm->nrow = m.nrow;
        fm->ncol = m.ncol;
        #ifdef DEBUG
            printf("downloaded(%d) vector, name: %s (%d:%d)\n",k_all, fm->name, fm->nrow, fm->ncol);
        #endif            
        MessageCont_add(mes_cont, fm);
    }
    printf("exit!\n");
    close(sock_desc);  
    pthread_exit(NULL);
}

typedef struct {
   MessageCont *mc;
   pthread_t t;
} ServerThread;



MessageCont* run_server(int port, pthread_t *t) {
    args_s args;
    MessageCont *mc = MessageContCreate();
    args.mc = mc;
    args.port = port;
    
    printf("0\n"); 
    
//    ServerThread *st = (ServerThread*)malloc(sizeof(ServerThread));
//    st->mc = mc;
//    st->t = (pthread_t*) malloc(sizeof(pthread_t)); 
//    printf("%d\n", &st->t);
    pthread_create( t, NULL, get_message, (void*)&args);
    sleep(100);
    return mc;
}


