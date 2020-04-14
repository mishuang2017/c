#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#ifdef SDP
#define PROTO_TEST      PROTO_SDP
#else
#define PROTO_TEST      IPPROTO_TCP
#endif

#define PORT 		4000

char *server_name = "localhost";
char *bind_name = NULL;
void *sdp_testing_client1(void *server_name);

int main(int argc, char *argv[])
{
	int i, c, n;
	int ret;
	int server, conn;
	int addrlen;
	int client_flag = 0;
	int server_flag = 0;
	struct sockaddr_in sin;
	struct sockaddr_in pin;
	pthread_attr_t attr;
	pthread_t t_client;
	char buf[10];
	int num;
	struct hostent *hp;

	while ((c = getopt(argc, argv, "b:sc:")) != -1)
		switch (c) {
			case 's':
				server_flag = 1;
				break;
			case 'c':
				client_flag = 1;
				server_name = optarg;
				break;
			case 'b':
				bind_name = optarg;
				break;
			default:
				exit(1);
		}


	if (client_flag == 0 && server_flag == 0) {
		perror("Please specify the server or the client");
		exit(1);
	}

	if (client_flag == 1) {
		/* create client thread */
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		if ((pthread_create(&t_client, &attr, sdp_testing_client1,
		    (void*)server_name)) != 0) {
			perror("pthread_create() error");
			exit(1);
		}

		pthread_attr_destroy(&attr);
		pthread_join(t_client, NULL);
	}

	if (server_flag == 1) {
/* 		printf("Server starting\n"); */
		if ((server = socket(AF_INET, SOCK_STREAM, PROTO_TEST)) == -1) {
			perror("server socket() error, %d, %d", server, errno);
			exit(1);
		}

		memset(&sin, 0, sizeof(sin));
		if (bind_name != NULL) {
			if ((hp = gethostbyname(bind_name)) == 0) {
				perror("client gethostbyname() error");
				exit(1);
			}
			sin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
			printf("bind to %s\n", bind_name);
		} else {
			sin.sin_addr.s_addr = INADDR_ANY;
			printf("bind to INADDR_ANY\n");
		}
		sin.sin_family = AF_INET;
		sin.sin_port = htons(PORT);
		if (bind(server, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
			perror("server bind() error");
			exit(1);
		}

		if (listen(server, 5) == -1) {
			perror("server listen() error");
			exit(1);
		}

/* 		printf("Server Socket successfully created!\n"); */

		addrlen = sizeof(pin); 
		if ((conn = accept(server, (struct sockaddr *) &pin, 
		    &addrlen)) == -1) {
			perror("server accept() error");
			exit(1);
		}

/* 		printf("Server: new connection accepted remote port=%d\n", */
/* 		    htons(pin.sin_port)); */

		memset(buf, 0, 10);
		n = recv(conn, buf, 10, 0);
		if ( n == -1 )
			perror("recv");
		else
			printf("recv() %s, return %d\n", buf, n);

		close(conn);
		close(server);
	}
}

void *sdp_testing_client1(void *server_name)
{
	int i, client, n;
	struct sockaddr_in sin;
	struct sockaddr_in pin;
	struct hostent *hp;
	int reuseaddr = 1;

/* 	printf("Client starting\n"); */

	if ((hp = gethostbyname(server_name)) == 0) {
		perror("client gethostbyname() error");
		exit(1);
	}

	memset(&pin, 0, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	pin.sin_port = htons(PORT);

	if ((client = socket(AF_INET, SOCK_STREAM, PROTO_TEST)) == -1) {
		perror("client socket() error");
		exit(1);
	}

        setsockopt(client, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
            sizeof (reuseaddr));

	memset(&sin, 0, sizeof(sin));
	if (bind_name != NULL) {
		if ((hp = gethostbyname(bind_name)) == 0) {
			perror("client gethostbyname() error");
			exit(1);
		}
		sin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
		printf("bind to %s\n", bind_name);
	} else {
		sin.sin_addr.s_addr = INADDR_ANY;
		printf("bind to INADDR_ANY\n");
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	if (bind(client, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
		perror("client bind() error");
		exit(1);
	}

/* 	printf("sizeof(pin) = %d\n", sizeof(pin)); */
	if (connect(client, (struct sockaddr *)&pin, sizeof(pin))
	    == -1) {
		perror("client connect() error");
		exit(1);
	}
/* 	printf("Client: connected to port %d\n", htons(pin.sin_port)); */

	if ((n = send(client, "bcd", 3, NULL)) == -1)
		perror("send");
	else
		printf("send() return %d\n", n);

	close(client);

	pthread_exit(NULL);
}
