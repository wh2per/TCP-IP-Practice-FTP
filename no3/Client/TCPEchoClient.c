#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */

#define RCVBUFSIZE 100 /* Size of receive buffer */
#define NAMESIZE 256
#define FILEBUFSIZE 1024

void DieWithError(char *errorMessage); /* Error handling function */

int main(int argc, char *argv[]) {
	int sock; /* Socket descriptor */
	struct sockaddr_in echoServAddr; /* Echo server address */
	unsigned short echoServPort; /* Echo server port */
	char *servIP; /* Server IP address (dotted quad) */
	char *echoString; /* String to send to echo server */
	char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    char fileBuffer[FILEBUFSIZE];
    
	unsigned int echoStringLen; /* Length of string to echo */
	int bytesRcvd, totalBytesRcvd; /* Bytes read in single recv() and total bytes read */
    int MsgSize=0;
    FILE* put;
    FILE* get;
    char* FileName;
    int FileSize = 0;
    int i=0;
    int recvSize=0;
    
    FileName = (char*)malloc(sizeof(char)*NAMESIZE);
	servIP = (char*)malloc(sizeof(char)*20);
	echoString = (char*)malloc(sizeof(char)*RCVBUFSIZE);
	
	printf("server IP : ");
	scanf("%s",servIP); /* First arg: server IP address (dotted quad) */
	printf("Port : ");
	scanf("%hu",&echoServPort); /* Use given port, if any */
	/* 7 is the well-known port for the echo service */
	/* Create a reliable, stream socket using TCP */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");
    
	/* Construct the server address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET; /* Internet address family */
	echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
	echoServAddr.sin_port = htons(echoServPort); /* Server port */
	/* Establish the connection to the echo server */
	if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError("connect() failed");
    
    /* hello hi */
    send(sock, "hello", 6, 0);
    printf("msg -> hello\n");
    MsgSize = recv(sock, echoBuffer, RCVBUFSIZE-1, 0);
    echoBuffer[MsgSize] = '\0';
    printf("msg <- %s\n",echoBuffer);
    /*------------------*/
   
    for(;;){
        /* Send the string to the server */
        printf("msg -> ");
        scanf("%s",echoString);
        if(!strcmp(echoString, "\\quit") ) break;
        
        /*FTP Action*/
        if(!strcmp(echoString, "FT") ){
            printf("Welcome to Socket FT Client!\n");
            echoStringLen = strlen(echoString);
            if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
                DieWithError("send() sent a different number of bytes than expected");
            for(;;){        // FTP Action
                printf("ftp command [ p)ut  g)et  l)s  r)ls  e)xit ] -> ");
                scanf("%s",echoString);
                echoStringLen = strlen(echoString); /* Determine input length */
                if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
                    DieWithError("send() sent a different number of bytes than expected");
                else if(!strcmp(echoString, "p") ){     // Put Action
                    printf("filename to put to server -> ");
                    scanf("%s",FileName);
                    if((put = fopen(FileName, "r"))==NULL)
                       printf("That file is not here!\n");
                    else{
                        fseek(put, 0, SEEK_END);    // file size check
                        FileSize = ftell(put);
                        sprintf(echoString, "%d", FileSize);
                        send(sock, echoString, RCVBUFSIZE, 0);
                        
                        if((MsgSize = recv(sock, echoBuffer, RCVBUFSIZE, 0))<0)      // file ack check
                            DieWithError("ACK recv failed!");
                        echoBuffer[MsgSize] = '\0';
                        if(!strcmp(echoBuffer, "FILE_ACK1")){
                            send(sock, FileName,NAMESIZE,0);
                        }else
                            printf("File AcK does not come to me!\n");
                        
                        if((MsgSize = recv(sock, echoBuffer, RCVBUFSIZE, 0))<0)      // file ack check
                            DieWithError("READY recv failed!");
                        echoBuffer[MsgSize] = '\0';
                        if(!strcmp(echoBuffer, "FILE_ACK2")){
                            printf("Sending => ####################\n");
                            i=0;
                            fseek(put, 0, SEEK_SET);
                            while(!feof(put)){
                                fileBuffer[i++] = fgetc(put);
                                if(i>=FILEBUFSIZE){
                                    fileBuffer[i] = '\0';
                                    if(send(sock, fileBuffer, FILEBUFSIZE, 0) == -1)
                                        DieWithError("file send failed!1");
                                    i=0;
                                }
                            }
                            fileBuffer[i-1] = '\0';
                            if(send(sock, fileBuffer, FILEBUFSIZE, 0) == -1)
                                DieWithError("file send failed!2");
                            else{
                                fclose(put);
                                MsgSize = recv(sock, echoBuffer, RCVBUFSIZE, 0);
                                echoBuffer[MsgSize] = '\0';
                                if(!strcmp(echoBuffer, "FILE_COMPLETE"))
                                    printf("%s(%d bytes) uploading success to 127.0.0.1\n",FileName,FileSize);
                            }
                        }
                    }
                }
                else if(!strcmp(echoString, "g") ){     // get Action
                    printf("filename to get from server -> ");
                    scanf("%s",FileName);
                    send(sock, FileName,NAMESIZE,0);
                    
                    if ((MsgSize = recv(sock, echoBuffer, RCVBUFSIZE, 0)) < 0)
                        DieWithError("recv() failed");
                    FileSize = atoi(echoBuffer);
                    
                    send(sock, "FILE_ACK", RCVBUFSIZE, 0);
                    
                    printf("Getting => ####################\n");
                    
                    recvSize=0;
                    get = fopen(FileName,"w");
                    while(recvSize<=FileSize){
                        if ((MsgSize = recv(sock, fileBuffer, FILEBUFSIZE, 0)) < 0)
                            DieWithError("recv() failed");
                        else{
                            recvSize+=MsgSize;
                            fileBuffer[MsgSize] = '\0';
                            fprintf(get,"%s",fileBuffer);
                        }
                    }
                    fclose(get);
                    printf("%s(%d bytes) downloading success to 127.0.0.1\n",FileName,FileSize);
                }
                else if(!strcmp(echoString, "l") )      // ls Action
                    system("ls");
                else if(!strcmp(echoString, "r") ){     // rls Action
                    if ((MsgSize = recv(sock, echoBuffer, RCVBUFSIZE, 0)) < 0)
                        DieWithError("recv() failed");
                    FileSize = atoi(echoBuffer);
                    if((send(sock, "RLS_ACK", RCVBUFSIZE, 0))==-1)
                        DieWithError("rls_ack send error");
                    recvSize=0;
                    while(recvSize<FileSize){
                        if((MsgSize = recv(sock, echoBuffer, RCVBUFSIZE, 0))<0){
                            DieWithError("recv() failed");
                        }else{
                            recvSize += MsgSize;
                            echoBuffer[MsgSize] = '\0';
                            printf("%s",echoBuffer);
                        }
                    }
                }
                else if(!strcmp(echoString, "e") ) break;
                else
                    printf("Wrong Input!\n");
            }
        }
        else{   /* Receive the same string back from the server */
            // Message Action
            echoStringLen = strlen(echoString); /* Determine input length */
            if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
                DieWithError("send() sent a different number of bytes than expected");
            totalBytesRcvd = 0;
            printf("msg <- "); /* Setup to print the echoed string */
            while (totalBytesRcvd < echoStringLen) {
                /* Receive up to the buffer size (minus 1 to leave space for a null terminator) bytes from the sender */
                if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
                    DieWithError("recv() failed or connection closed prematurely");
                totalBytesRcvd += bytesRcvd; /* Keep tally of total bytes */
                echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */
                printf("%s",echoBuffer); /* Print the echo buffer */
            }
            printf("\n"); /* Print a final linefeed */
        }
    }
	close(sock);
	exit(0);
}
