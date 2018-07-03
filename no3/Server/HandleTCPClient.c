#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h> /* for close() */
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#define RCVBUFSIZE 100 /* Size of receive buffer */
#define NAMESIZE 256
#define FILEBUFSIZE 1024

void DieWithError(char *errorMessage); /* Error handling function */

void HandleTCPClient(int clntSocket) {
    int fd;
    FILE* rls;
    FILE* put;
    FILE* get;
    
    char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    char fileBuffer[FILEBUFSIZE];
	int recvMsgSize; /* Size of received message */
    int i=0;
    int state = 0;
    char* FileName;
    int FileSize = 0;
    int recvSize=0;
    
    FileName = (char*)malloc(sizeof(char)*NAMESIZE);
    fd = open("echo_history.log", O_RDWR | O_APPEND | O_CREAT, 0644);
    if(fd==-1)
        puts("open() error\n");
    
	/* Receive message from client */
	if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed");
    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0) { /* zero indicates end of transmission */
        echoBuffer[recvMsgSize] = '\0';
        /* FTP Action */
        if(!strcmp(echoBuffer, "FT"))
            state=1;
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
            if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
                DieWithError("send() failed");
            printf("msg -> %s\n", echoBuffer);
        }
        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
    }
    close(fd);
	close(clntSocket); /* Close client socket */
}
