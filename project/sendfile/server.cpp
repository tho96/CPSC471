
#include <stdio.h>      /* Contains common I/O functions */
#include <sys/types.h>  /* Contains definitions of data types used in system calls */
#include <sys/socket.h> /* Includes definitions of structures needed for sockets */
#include <netinet/in.h> /* Contains constants and structures needed for internet domain addresses. */
#include <unistd.h>     /* Contains standard unix functions */
#include <stdlib.h>     /* For atoi() and exit() */
#include <string.h> 	/* For memset() */
struct file{
	int size;
	char * buffer;
};
int main(int argc, char** argv)
{
	/* Holds data for file */
	file destinationFile;

	/* The port number */	
	int port = -1;

	/* The integer to store the file descriptor number
	 * which will represent a socket on which the server
	 * will be listening
	 */
	int listenfd = -1;
	
	/* The file descriptor representing the connection to the client */
	int connfd = -1;
	
	/* The size of the filename buffer */
	#define FILENAME_BUFF_SIZE 100

	/* The buffer to store the name of transfered file */
	char filenameBuff[FILENAME_BUFF_SIZE];


	/* The structures representing the server and client
	 * addresses respectively
	 */
	sockaddr_in serverAddr, cliAddr;
	
	/* Stores the size of the client's address */
	socklen_t cliLen = sizeof(cliAddr);	
	
	/* Make sure the user has provided the port number to
	 * listen on
	 */
	if(argc < 2)
	{
		/* Report an error */
		fprintf(stderr, "USAGE: %s <PORT #>", argv[0]);
		exit(-1);	
	}
	
	/* Get the port number */
	port = atoi(argv[1]);
	
	/* Make sure that the port is within a valid range */
	if(port < 0 || port > 65535)	
	{
		fprintf(stderr, "Invalid port number\n");
		exit(-1);
	} 
	
	/* Print the port number */	
	fprintf(stderr, "Port  = %d\n", port); 	
	
	/* Create a socket that uses
	 * IPV4 addressing scheme (AF_INET),
	 * Supports reliable data transfer (SOCK_STREAM),
	 * and choose the default protocol that provides
	 * reliable service (i.e. 0); usually TCP
	 */
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(-1);
	}
	
	
	/* Set the structure to all zeros */
	memset(&serverAddr, 0, sizeof(serverAddr));
		
	/* Convert the port number to network representation */	
	serverAddr.sin_port = htons(port);
	
	/* Set the server family */
	serverAddr.sin_family = AF_INET;
	
	/* Retrieve packets without having to know your IP address,
	 * and retrieve packets from all network interfaces if the
	 * machine has multiple ones
	 */
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	/* Associate the address with the socket */
	if(bind(listenfd, (sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("bind");
		exit(-1);
	}
	
	/* Listen for connections on socket listenfd.
	 * allow no more than 100 pending clients.
	 */
	if(listen(listenfd, 100) < 0)
	{
		perror("listen");
		exit(-1);
	}
	
	/* Wait forever for connections to come */
	while(true)
	{
		
		fprintf(stderr, "\nWaiting for somebody to connect on port %d\n", port);
				
		/* A structure to store the client address */
		if((connfd = accept(listenfd, (sockaddr *)&cliAddr, &cliLen)) < 0)
		{
			perror("accept");
			exit(-1);
		}
		fprintf(stderr, "Connected!\n\n");	

		/* Get file size from client */
		if((read(connfd, &destinationFile.size, 4)) < 0)
		{
			perror("read");	
			exit(-1);
		}
		/* Read filename from client */
		if((read(connfd, filenameBuff, FILENAME_BUFF_SIZE)) < 0)
		{
			perror("read");	
			exit(-1);
		}
		/* Allocate memory for file buffer */
		destinationFile.buffer = new char[destinationFile.size+1];
		//fileBuffer[destinationFile.size] = '\0';

		/* Get file contents from client */
		int numReceived = 0, totalReceived = 0;
		while(totalReceived != destinationFile.size)
		{
			if((numReceived = read(connfd, destinationFile.buffer + totalReceived, destinationFile.size - totalReceived)) < 0)
			{
				perror("read");
				exit(-1);
			}
			totalReceived += numReceived;
		}
		fprintf(stderr, "File Name: %s\n", filenameBuff); 
		fprintf(stderr, "File Size: %i bytes\n", destinationFile.size); 
		fprintf(stderr, "Bytes written: %i bytes\n", totalReceived); 
		if(totalReceived == destinationFile.size)
			fprintf(stderr, "%s successfully transfered\n", filenameBuff); 
		else
			fprintf(stderr, "Error during file transfer\n"); 
	


		FILE * destFile;
		destFile = fopen(filenameBuff, "wb");
		fwrite(destinationFile.buffer, 1 , destinationFile.size , destFile);
		fclose(destFile);
		

		/* Close the socket */
		close(connfd);
	}			
	return 0;
}
