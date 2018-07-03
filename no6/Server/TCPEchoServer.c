#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */
#include <pthread.h>
#include <fcntl.h>

#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define BUFSIZE 32
#define RCVBUFSIZE 100 /* Size of receive buffer */
#define STRSIZEBUF 4
#define NAMESIZE 256
#define FILEBUFSIZE 1024

struct ThreadArgs{
    int clntSock; /* Socket descriptor for client */
    int id;
};

void DieWithError(char *errorMessage); /* Error handling function */
void HandleTCPClient(int clntSocket, int id); /* TCP client handling function */
void *ThreadMain(void* threadArgs);

int clntSock[100]; /* Socket descriptor for client */
int cc;

int main(int argc, char *argv[]){
	int servSock; /* Socket descriptor for server */
	struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
	unsigned short echoServPort; /* Server port */
    unsigned int clntLen; /* Length of client address data structure */
	echoServPort = 9999; /* First arg: local port */
    int id=0;
    pthread_t threadID[100];
   
	/* Create socket for incoming connections */
	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");
	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET; /* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr.sin_port = htons(echoServPort); /* Local port */
	/* Bind to the local address */
	if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError("bind() failed");
	/* Mark the socket so it will listen for incoming connections */
	if (listen(servSock, MAXPENDING) < 0)
		DieWithError("listen() failed");
    
    struct ThreadArgs *threadArgs = (struct ThreadArgs*)malloc(sizeof(struct ThreadArgs));
    if(threadArgs == NULL)
        DieWithError("malloc() failed");
    
    
	for (;;) /* Run forever */
	{
		/* Set the size of the in-out parameter */
		clntLen = sizeof(echoClntAddr);
		/* Wait for a client to connect */
		if ((clntSock[cc] = accept(servSock, (struct sockaddr *) &echoClntAddr,&clntLen)) < 0)
            DieWithError("accept() failed");
        /* clntSock is connected to a client! */
        printf("client ip : %s\n", inet_ntoa(echoClntAddr.sin_addr));
        printf("port %hu\n", echoServPort);
        int id = cc++;
        
        threadArgs->clntSock = clntSock[id];
        threadArgs->id = id;
        
        int returnValue = pthread_create(&threadID[id], NULL, ThreadMain, threadArgs);
        if(returnValue!=0){
            DieWithError("pthread_create() failed");
            printf("with thread %ld\n",(long int)threadID);
        }
	}
    exit(0);
	/* NOT REACHED */
}

void broadcast(char *msg, int sender){
    for(int i=0; i<cc; i++){
        if(clntSock[i] > 0 && i != sender)
            send(clntSock[i],msg,strlen(msg),0);
    }
}

void HandleTCPClient(int clntSocket,int id) {
    int fd;
    FILE* rls;
    FILE* put;
    FILE* get;
    char *echoBuffer; /* Buffer for echo string */
    char fileBuffer[FILEBUFSIZE];
    int recvMsgSize; /* Size of received message */
    int i=0;
    int state = 0;
    char* FileName;
    int FileSize = 0;
    int recvSize=0;
    
    FileName = (char*)malloc(sizeof(char)*NAMESIZE);
    echoBuffer = (char*)malloc(sizeof(char)*RCVBUFSIZE);
    
    fd = open("echo_history.log", O_RDWR | O_APPEND | O_CREAT, 0644);
    if(fd==-1)
        puts("open() error\n");
    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0) { /* zero indicates end of transmission */
        /* Echo message back to client */
        echoBuffer[recvMsgSize] = '\0';
        
        /* FTP Action */
        if(!strcmp(echoBuffer, "FT")){
            state=1;
            if (send(clntSocket, "#", RCVBUFSIZE, 0) == -1)
                DieWithError("send() failed");
        }
        else if(!strcmp(echoBuffer, "p") && state==1){
            if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
                DieWithError("recv() failed");
            FileSize = atoi(echoBuffer);
            
            if (send(clntSocket, "FILE_ACK1", RCVBUFSIZE, 0) == -1)
                DieWithError("send() failed");
            recvMsgSize = recv(clntSocket, FileName, NAMESIZE, 0);
            FileName[recvMsgSize] = '\0';
            
            if (send(clntSocket, "FILE_ACK2", RCVBUFSIZE, 0) == -1)
                DieWithError("send() failed");
            
            recvSize=0;
            put = fopen(FileName,"w");
            while(recvSize<=FileSize){
                if((recvMsgSize = recv(clntSocket, fileBuffer, FILEBUFSIZE, 0))<0)
                    DieWithError("file recv faild");
                else{
                    recvSize+=recvMsgSize;
                    fileBuffer[recvMsgSize] = '\0';
                    fprintf(put,"%s",fileBuffer);
                }
            }
            fclose(put);
            send(clntSocket, "FILE_COMPLETE", RCVBUFSIZE, 0);
        }else if(!strcmp(echoBuffer, "g") && state==1){     // get Action
            recvMsgSize = recv(clntSocket, FileName, NAMESIZE, 0);
            FileName[recvMsgSize] = '\0';
            if((get = fopen(FileName, "r"))==NULL)
                printf("That file is not here!\n");
            
            fseek(get, 0, SEEK_END);    // file size check
            FileSize = ftell(get);
            sprintf(echoBuffer, "%d", FileSize);
            send(clntSocket, echoBuffer, RCVBUFSIZE, 0);
            
            recvMsgSize = recv(clntSocket, echoBuffer,RCVBUFSIZE, 0);
            if(!strcmp(echoBuffer, "FILE_ACK")){
                i=0;
                fseek(get, 0, SEEK_SET);
                while(!feof(get)){
                    fileBuffer[i++] = fgetc(get);
                    if(i>=FILEBUFSIZE){
                        fileBuffer[i] = '\0';
                        if(send(clntSocket, fileBuffer, FILEBUFSIZE, 0) == -1)
                            DieWithError("file send failed!");
                        i=0;
                    }
                }
                fileBuffer[i-1] = '\0';
                if(send(clntSocket, fileBuffer, FILEBUFSIZE, 0) == -1)
                    DieWithError("file send failed!");
            }
            fclose(get);
        }else if(!strcmp(echoBuffer, "r") && state==1 ){    // rls Action
            system("ls >rls.txt");
            rls = fopen("rls.txt","r");
            
            fseek(rls, 0, SEEK_END);    // file size check
            FileSize = ftell(rls);
            sprintf(echoBuffer, "%d", FileSize);
            if((send(clntSocket, echoBuffer, RCVBUFSIZE, 0))==-1)
                DieWithError("size send error");
            
            recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0);
            
            if(!strcmp(echoBuffer, "RLS_ACK")){
                i=0;
                fseek(rls, 0, SEEK_SET);
                while(!feof(rls)){
                    echoBuffer[i++] = fgetc(rls);
                    if(i==100){
                        echoBuffer[i] = '\0';
                        if (send(clntSocket, echoBuffer, RCVBUFSIZE, 0) == -1)
                            DieWithError("send() failed");
                        i=0;
                    }
                }
                echoBuffer[i-1] = '\0';
                if (send(clntSocket, echoBuffer, RCVBUFSIZE, 0) == -1)
                    DieWithError("send() failed");
            }
            fclose(rls);
        }else if(!strcmp(echoBuffer, "l") && state==1){     // ls Action
            
        }else if(!strcmp(echoBuffer, "e") && state==1){     // exit Action
            state = 0;
        }else{  /* Echo message back to client */
            printf("msg <- %s\n",echoBuffer);
        
            if(write(fd,echoBuffer,recvMsgSize)==-1)
                puts("write() Error!\n");
            echoBuffer[recvMsgSize+1] = '\n';
            broadcast(echoBuffer,id);
        }
            /* See if there is more data to receive */
            if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
                DieWithError("recv() failed");
    }
    close(fd);
    close(clntSocket); /* Close client socket */
}

void *ThreadMain(void* threadArgs){
    pthread_detach(pthread_self());

    int clntSock = ((struct ThreadArgs*)threadArgs)->clntSock;
    int id = ((struct ThreadArgs*)threadArgs)->id;
    char Buffer[BUFSIZE];
    int MsgSize=0;
    
    /* hello hi */
    MsgSize = recv(clntSock, Buffer, BUFSIZE, 0);
    Buffer[MsgSize] = '\0';
    printf("msg <- %s\n",Buffer);
    send(clntSock,"hi",3,0);
    printf("msg -> hi\n");
    /*------------------*/
    HandleTCPClient(clntSock,id);
    
    return 0;
}
