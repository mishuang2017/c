// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define PORT	 8080 
#define MAXLINE 1024 

// Driver code 
int main(int argc, char *argv[])
{
	int c;
	int sockfd; 
	char buffer[MAXLINE]; 
	char hello[50] = "Hello from client"; 
	struct sockaddr_in	 servaddr; 
	char *server_name = "localhost";
	extern char *optarg;
	int i, t = 1;
	int interval = 1;
	int verbose = 0;

	while ((c = getopt(argc, argv, "c:t:i:v")) != -1) {
                switch (c) {
                        case 'c':
				server_name = optarg;
				break;
                        case 't':
				sscanf(optarg, "%d", &t);
				break;
			case 'i':
				sscanf(optarg, "%d", &interval);
				break;
			case 'v':
				verbose = 1;
				break;
		}
	}

	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 

	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(PORT); 
	servaddr.sin_addr.s_addr = inet_addr(server_name);

	int n, len; 
	
	for (i = 0; i < t; i++) {
		char str[100];
		snprintf(str, 100, "%s %d\0", hello, i);
		sendto(sockfd, (const char *)str, strlen(str), 
			MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
				sizeof(servaddr)); 
		if (verbose)
			printf("Hello message sent %d.\n", i + 1); 
			
#if 0
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
					MSG_WAITALL, (struct sockaddr *) &servaddr, 
					&len); 
		buffer[n] = '\0'; 
		if (verbose)
			printf("Server : %s\n", buffer); 
#endif
		sleep(interval);
	}

	close(sockfd); 
	return 0; 
} 
