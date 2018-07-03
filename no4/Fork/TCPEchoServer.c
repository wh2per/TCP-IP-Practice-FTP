#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */

#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define BUFSIZE 32

void DieWithError(char *errorMessage); /* Error handling function */
void HandleTCPClient(int clntSocket); /* TCP client handling function */
int main(int argc, char *argv[]){
	int servSock; /* Socket descriptor for server */
	int clntSock; /* Socket descriptor for client */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr; /* Client address */
	unsigned short echoServPort; /* Server port */
	unsigned int clntLen; /* Length of client address data structure */
    char Buffer[BUFSIZE];
    int MsgSize=0;
    pid_t ProcessID = 0;
    unsigned int childProcCount = 0;
	echoServPort = 9999; /* First arg: local port */
    
    
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
    
	for (;;) /* Run forever */
	{
		/* Set the size of the in-out parameter */
		clntLen = sizeof(echoClntAddr);
		/* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr,&clntLen)) < 0)
            DieWithError("accept() failed");
        ProcessID = fork();
            
        if(ProcessID<0)
            DieWithError("fork() failed");
        else if(ProcessID == 0){
            /* clntSock is connected to a client! */
            printf("client ip : %s\n", inet_ntoa(echoClntAddr.sin_addr));
            printf("port %hu\n", echoServPort);
            
            /* hello hi */
            MsgSize = recv(clntSock, Buffer, BUFSIZE, 0);
            Buffer[MsgSize] = '\0';
            printf("msg <- %s\n",Buffer);
            send(clntSock,"hi",3,0);
            printf("msg -> hi\n");
            /*------------------*/
            close(servSock);
            HandleTCPClient(clntSock);
            exit(0);
        }
	}
	/* NOT REACHED */
}
