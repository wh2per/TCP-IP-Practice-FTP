#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h> /* for close() */
#include <fcntl.h>

#define RCVBUFSIZE 32 /* Size of receive buffer */
void DieWithError(char *errorMessage); /* Error handling function */
void HandleTCPClient(int clntSocket) {
    int fd;
    char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
	int recvMsgSize; /* Size of received message */
    
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
        echoBuffer[recvMsgSize+1] = '\n';
        printf("Received: %s\n",echoBuffer);
        if(write(fd,echoBuffer,recvMsgSize+2)==-1)
            puts("write() Error!\n");
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
			DieWithError("send() failed");
		/* See if there is more data to receive */
		if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
			DieWithError("recv() failed");
	}
    close(fd);
	close(clntSocket); /* Close client socket */
}
