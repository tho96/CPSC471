
#include <stdio.h>      /* Contains common I/O functions */
#include <sys/types.h>  /* Contains definitions of data types used in system calls */
#include <sys/socket.h> /* Includes definitions of structures needed for sockets */
#include <netinet/in.h> /* Contains constants and structures needed for internet domain addresses. */
#include <unistd.h>     /* Contains standard unix functions */
#include <stdlib.h>     /* For atoi() and exit() */
#include <string.h> 	/* For memset() */
#include <arpa/inet.h>  /* For inet_pton() */
#include <iostream>

struct file{
	int size;
	char * buffer;
};
int main(int argc, char** argv)
{
	/* File name */
	char * filename = argv[3];

	/* File data being sent to server */
	file sourceFile;

	FILE * source = NULL;
    source = fopen(filename,"r");
    
    fseek(source,0,SEEK_END);
    /* Get file size of input file */
    sourceFile.size = ftell(source);
    fseek(source, 0, SEEK_SET);

    /* Allocate buffer size for file */
	sourceFile.buffer = new char[sourceFile.size+1];
	if (!sourceFile.buffer)
	{
		fprintf(stderr, "Memory error");
		fclose(source);
		exit(-1);
	}
	/* Read file into buffer */
	fread(sourceFile.buffer,1,sourceFile.size,source);
    fclose(source);
	
	/* The port number */	
	int port = -1;
	
	/* The file descriptor representing the connection to the client */
	int connfd = -1;
	
	/* The size of the filename buffer */
	#define FILENAME_BUFF_SIZE 100

	/* The buffer to store filename sent back from server */
	char filenameBuff[FILENAME_BUFF_SIZE];
		
	/* The structures representing the server address */
	sockaddr_in serverAddr;
	
	/* Stores the size of the client's address */
	socklen_t servLen = sizeof(serverAddr);	
	
	if(argc < 4)
	{
		/* Report an error */
		fprintf(stderr, "USAGE: %s <HOST IP> <PORT #> <FILENAME>", argv[0]);
		exit(-1);	
	}
	/* Get the port number */
	port = atoi(argv[2]);
	
	/* Make sure that the port is within a valid range */
	if(port < 0 || port > 65535)	
	{
		fprintf(stderr, "Invalid port number\n");
		exit(-1);
	} 
	
	/* Connect to the server */
	if((connfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(-1);
	}
		
	/* Set the structure to all zeros */
	memset(&serverAddr, 0, sizeof(serverAddr));
		
	/* Set the server family */
	serverAddr.sin_family = AF_INET;
	
	/* Convert the port number to network representation */	
	serverAddr.sin_port = htons(port);
	
	
	/**
	 * Convert the IP from the presentation format (i.e. string)
	 * to the format in the serverAddr structure.
	 */
	if(!inet_pton(AF_INET, argv[1], &serverAddr.sin_addr))
	{
		perror("inet_pton");
		exit(-1);
	}
	
	
	/* Lets connect to the client. This call will return a socket used 
	 * used for communications between the server and the client.
	 */
	if(connect(connfd, (sockaddr*)&serverAddr, sizeof(sockaddr))<0)
	{
		perror("connect");
		exit(-1);
	}	
	/* Send file size to server before sending file contents */
	if(write(connfd, &sourceFile.size, 4) < 0)
	{
		perror("write");
		exit(-1);
	}
	/* Send file name to server */
	if(write(connfd, filename, FILENAME_BUFF_SIZE) < 0)
	{
		perror("write");
		exit(-1);
	}

	int numSent = 0; 
	int totalSent = 0;
	while(totalSent != sourceFile.size)
	{
		if((numSent = write(connfd, sourceFile.buffer + totalSent, sourceFile.size - totalSent)) < 0)
		{
			perror("write");
			exit(-1);
		}

		totalSent += numSent;
	}

	if(totalSent == sourceFile.size)
		fprintf(stderr, "%s Successfully uploaded\n\n", filename);

	/* Close the connection socket */	
	close(connfd);
		
	return 0;
}
