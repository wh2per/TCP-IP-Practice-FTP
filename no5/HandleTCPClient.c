#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <unistd.h> /* for close() */
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define RCVBUFSIZE 100 /* Size of receive buffer */
void DieWithError(char *errorMessage); /* Error handling function */
void HandleTCPClient(int servSocket) {
    int fd;
    char *echoBuffer; /* Buffer for echo string */
	int recvMsgSize; /* Size of received message */
    struct sockaddr_in echoClntAddr; /* Client address */
    
    unsigned int clntLen;
    clntLen = sizeof(echoClntAddr);
    
    echoBuffer = (char*)malloc(sizeof(char)*RCVBUFSIZE);
    
    fd = open("echo_history.log", O_RDWR | O_APPEND | O_CREAT, 0644);
    if(fd==-1)
        puts("open() error\n");
	/* Receive message from client */
	if ((recvMsgSize = recvfrom(servSocket, echoBuffer, RCVBUFSIZE, 0,(struct sockaddr *)&echoClntAddr,&clntLen)) < 0)
		DieWithError("recv() failed");
	/* Send received string and receive again until end of transmission */
	while (recvMsgSize > 0) { /* zero indicates end of transmission */
		/* Echo message back to client */
        echoBuffer[recvMsgSize] = '\0';
        printf("msg <- %s\n",echoBuffer);
        if(write(fd,echoBuffer,recvMsgSize)==-1)
            puts("write() Error!\n");
        if (sendto(servSocket, echoBuffer, recvMsgSize, 0,(struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
			DieWithError("send() failed");
        printf("msg -> %s\n", echoBuffer);
		/* See if there is more data to receive */
		if ((recvMsgSize = recvfrom(servSocket, echoBuffer, RCVBUFSIZE, 0,(struct sockaddr *)&echoClntAddr, &clntLen)) < 0)
			DieWithError("recv() failed");
	}
    close(fd);
	close(servSocket); /* Close client socket */
}
