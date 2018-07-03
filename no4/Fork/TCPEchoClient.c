#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */
#include <fcntl.h>
#define RCVBUFSIZE 100 /* Size of receive buffer */
void DieWithError(char *errorMessage); /* Error handling function */

int main(int argc, char *argv[]) {
	int sock; /* Socket descriptor */
	struct sockaddr_in echoServAddr; /* Echo server address */
	unsigned short echoServPort; /* Echo server port */
	char *servIP; /* Server IP address (dotted quad) */
	char *echoString; /* String to send to echo server */
	char *echoBuffer; /* Buffer for echo string */
	unsigned int echoStringLen; /* Length of string to echo */
	int bytesRcvd, totalBytesRcvd; /* Bytes read in single recv() and total bytes read */
    int MsgSize=0;
    servIP = (char*)malloc(sizeof(char)*20);
	echoString = (char*)malloc(sizeof(char)*RCVBUFSIZE);
	echoBuffer = (char*)malloc(sizeof(char)*RCVBUFSIZE);
    
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
        fflush(stdout);
        echoStringLen= read(0,echoString,RCVBUFSIZE);
        echoString[echoStringLen-1] = '\0';
        if(!strcmp(echoString, "\\quit") ) break;
        if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
            DieWithError("send() sent a different number of bytes than expected");
        /* Receive the same string back from the server */
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
        printf("\n");
    }
	close(sock);
	exit(0);
}
