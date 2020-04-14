#define _XPG4_2
#define __EXTENSIONS__

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
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
#include <sys/file.h>

#ifdef SDP
#define PROTO_TEST      PROTO_SDP
#else
#define PROTO_TEST      IPPROTO_TCP
#endif

#define PORT	4000
#define CLIENT	1
#define BUF_SIZE  10

char *server_name = "localhost";
void *sdp_testing_client1(void *server_name);

int main(int argc, char *argv[])
{
	int i, c;
	int ret;
	int server, conn = 0;
	unsigned int addrlen;
	int client_flag = 0;
	int server_flag = 0;
	int reuseaddr = 1;
	struct sockaddr_in sin;
	struct sockaddr_in pin;
	pthread_t t_client[3], server_rd, server_wr;
	char *buf, *buf2;
	char string[10];
	struct msghdr msg;
	struct iovec iov[2];

	while ((c = getopt(argc, argv, "sc:")) != -1)
		switch (c) {
			case 's':
				server_flag = 1;
				break;
			case 'c':
				client_flag = 1;
				server_name = optarg;
				break;
			default:
				exit(1);
		}

	printf("msg size = %d\n", sizeof (struct msghdr));

	if (client_flag == 0 && server_flag == 0) {
		perror("Please specify the server or the client");
		exit(1);
	}

	if (client_flag == 1) {
		for (i = 0; i < CLIENT; i ++) {
			sprintf(string, "%s/%d", server_name, i);

			if ((pthread_create(&t_client[i], NULL,
			    sdp_testing_client1, string)) != 0) {
				perror("pthread_create() error");
				exit(1);
			}

			pthread_join(t_client[i], NULL);
		}
		exit(0);
	}

	if (server_flag == 1) {
		if ((server = socket(AF_INET, SOCK_STREAM, PROTO_TEST)) == -1) {
			perror("server socket() error");
			exit(1);
		}

		setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
		    sizeof(reuseaddr));

		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(PORT);
		if (bind(server, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
			perror("server bind() error");
			exit(1);
		}

		printf("server = %d\n", server);

		if (listen(server, 5) == -1) {
			perror("server listen() error");
			exit(1);
		}

		addrlen = sizeof(pin);
		if ((conn = accept(server, (struct sockaddr *) &pin, &addrlen))
		    == -1) {
			perror("accept");
		}

		buf = calloc(1, sizeof(3));
		buf2 = calloc(1, sizeof(4));

		iov[0].iov_base = buf;
		iov[0].iov_len = 3;
		iov[1].iov_base = buf2;
		iov[1].iov_len = 4;

		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = iov;
		msg.msg_iovlen = 2;

		ret = recvmsg(conn, &msg, 0);
		if (ret == -1) {
			perror("recv");
		}
		printf("server recvmsg returns (%d)\n", ret);
		printf("buf = %s\n", buf);
		printf("buf2 = %s\n", buf2);

		printf("close\n");
		close(conn);
		close(server);
	}
}

void *sdp_testing_client1(void *arg)
{
	int i, client, ret;
	struct sockaddr_in pin;
	struct hostent *hp;
	char buf;
	pthread_t client_rd, client_wr;
	int n;
	char c;
	char bigbuf[BUF_SIZE];
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct iovec iov[2];
	void *ctlbuf;
	char *cm = "control";

	printf("Client starting with string %s\n", arg);

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

	printf("connect\n");
	if (connect(client, (struct sockaddr *)&pin, sizeof(pin)) == -1) {
		perror("client connect() error");
		exit(1);
	}
	printf("Client: socket created %d\n", htons(pin.sin_port));

	printf("sendmsg\n");

	iov[0].iov_len = 3;
	iov[0].iov_base = "sdp";
	iov[1].iov_len = 4;
	iov[1].iov_base = "eoib";

	memset(&msg, 0, sizeof(msg));

	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

/* 	ctlbuf = calloc(1, CMSG_SPACE(sizeof(cm))); */
/*         if (!ctlbuf) { */
/*                 printf("%s: calloc failed\n", __func__); */
/*                 return; */
/*         } */
/* 	msg.msg_control = ctlbuf; */
/* 	msg.msg_controllen = CMSG_SPACE(sizeof(cm)); */
/* 	cmsg = CMSG_FIRSTHDR(&msg); */
/* 	cmsg->cmsg_level = 16; */
/* 	cmsg->cmsg_type = 32; */
/* 	cmsg->cmsg_len = CMSG_LEN(7); */
/* 	memcpy(CMSG_DATA(cmsg), cm, sizeof(cm)); */

	ret = sendmsg(client, &msg, 0);
	if (ret == -1) {
		perror("sendmsg");
	}

	printf("sendmsg return %d\n", ret);

	printf("close\n");
	close(client);
}
