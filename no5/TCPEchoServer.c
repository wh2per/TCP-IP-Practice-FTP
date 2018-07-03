#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */


#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define BUFSIZE 32

void DieWithError(char *errorMessage); /* Error handling function */
void HandleTCPClient(int servSocket); /* TCP client handling function */
int main(int argc, char *argv[]){
	int servSock; /* Socket descriptor for server */
    
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr; /* Client address */
	unsigned short echoServPort; /* Server port */
	unsigned int clntLen; /* Length of client address data structure */
    char Buffer[BUFSIZE];
    int MsgSize=0;
    
    
	echoServPort = 9999; /* First arg: local port */
    
	/* Create socket for incoming connections */
	if ((servSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");
	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET; /* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr.sin_port = htons(echoServPort); /* Local port */
	/* Bind to the local address */
	if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError("bind() failed");
	
	for (;;) /* Run forever */
	{
		/* Set the size of the in-out parameter */
		clntLen = sizeof(echoClntAddr);
		
        /* hello hi */
        MsgSize = recvfrom(servSock, Buffer, BUFSIZE, 0,(struct sockaddr *)&echoClntAddr, &clntLen);
        Buffer[MsgSize] = '\0';
        /* clntSock is connected to a client! */
        printf("client ip : %s\n", inet_ntoa(echoClntAddr.sin_addr));
        printf("port %hu\n", echoServPort);
        printf("msg <- %s\n",Buffer);
        sendto(servSock,"hi",3,0,(struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr));
        printf("msg -> hi\n");
        /*------------------*/
        HandleTCPClient(servSock);
        close(servSock);
        exit(0);
	}
	/* NOT REACHED */
}
