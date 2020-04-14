/*
 * Copyright (c) 2014, Oracle and/or its affiliates. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/devpoll.h>

#ifdef SDP
#define	PROTO_TEST	PROTO_SDP
#else
#define	PROTO_TEST	IPPROTO_TCP
#endif

#define	MAX_FD		1000
#define	BUFFER_SIZE	65536

#define	MAXBUF	1000

extern int errno;

int debug = 0;

/* the default port number */
char *port = "4000";
char *bind_name = NULL;
char *version = "IPv4";

int run_time = 120;

typedef struct client_arg {
	struct addrinfo *c_res;
	struct addrinfo *c_bind_res;
} client_arg_t;

void
usage(void)
{
	printf("client usage: <b:c:hl:n:p:t:v:x:>\n"
	    "\t-b	specify hostname to bind, default value is NULL\n"
	    "\t-c	specify server name to connect\n"
	    "\t-h	print this usage\n"
	    "\t-l	lwp/thread number to be created, default value is 1\n"
	    "\t-n	packets number to be sent when debug level > 0\n"
	    "\t-p	port number, default value is 4000\n"
	    "\t-t	seconds to run, default value is 120 seconds\n"
	    "\t-v	IPv4 or IPv6, 4 or 6, default value is 4\n"
	    "\t-x	enable debug , default is disabled\n"
	    "\nserver usage: <b:hl:p:st:v:x:>\n"
	    "\t-b	specify hostname to bind, default value is NULL\n"
	    "\t-h	print this usage\n"
	    "\t-l	lwp/thread number to be created, default value is 1\n"
	    "\t-p	port number, default value is 4000\n"
	    "\t-s	server\n"
	    "\t-t	seconds to run, default value is 120 seconds\n"
	    "\t-v	IPv4 or IPv6, 4 or 6, default value is 4\n"
	    "\t-x	enable debug , default is disabled\n");

	exit(1);
}

int
sdp_client_connect(void *arg)
{
	int i, fd;
	struct addrinfo *ai;
	client_arg_t *c_arg;

	c_arg = (client_arg_t *)arg;

	ai = c_arg->c_res;
	if ((fd = socket(ai->ai_family, ai->ai_socktype, PROTO_TEST)) == -1) {
		perror("client socket() error");
		return (NULL);
	}

	ai = c_arg->c_bind_res;
	if (ai != NULL) {
		if (bind(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
			perror("client bind() error");
			return (NULL);
		}
	}

	ai = c_arg->c_res;
	if (connect(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
		perror("client connect() error");
		return (NULL);
	}

	printf("client fd: %d\n", fd);

	return (fd);
}

void
server_corrupt(char *port, int lwp, int v6)
{
	int i, fd, server, reuseaddr = 1;
	unsigned int addrlen;
	struct sockaddr_storage cliaddr;
	struct addrinfo hints, *res = NULL, *ressave = NULL;
	unsigned char buffer[BUFFER_SIZE];
	time_t start_time;		/* the start time */
	time_t current_time;		/* the current time */
	int fds[MAX_FD];

	int wfd;
	struct pollfd *pollfd;
	struct dvpoll dopoll;
	int result;
	int flags;

	printf("%s server starting... pid: %d\n", version, getpid());
	bzero(&hints, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((wfd = open("/dev/poll", O_RDWR)) < 0) {
		exit(-1);
	}
	pollfd = (struct pollfd* )malloc(sizeof(struct pollfd) * lwp);
	if (pollfd == NULL) {
		close(wfd);
		exit(-1);
	}
	if (getaddrinfo(bind_name, port, &hints, &res) != 0) {
		perror("server bind_name getaddrinfo() error");
		exit(1);
	}

	while (res != NULL) {
		if (v6 == 4 && res->ai_family == AF_INET) {
			ressave = res;
			break;
		}
		if (v6 == 6 && res->ai_family == AF_INET6) {
			ressave = res;
			break;
		}
		res = res->ai_next;
	}

	if (ressave == NULL) {
		printf("please specify correct hostname\n");
		exit(1);
	}

	if ((server = socket(v6 == 6 ? PF_INET6 : PF_INET, SOCK_STREAM,
	    PROTO_TEST)) == -1) {
		perror("server socket() error");
		exit(1);
	}

	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
	    sizeof (reuseaddr));

	if (bind(server, ressave->ai_addr, ressave->ai_addrlen) == -1) {
		perror("server bind() error");
		exit(1);
	}

	if (listen(server, 128) == -1) {
		perror("server listen() error");
		exit(1);
	}

	for (i = 0; i < lwp; i ++) {
		addrlen = sizeof (cliaddr);
		if ((fd = accept(server, (struct sockaddr *)&cliaddr,
		    &addrlen)) == -1) {
			perror("server accept() error");
			exit(1);
		}

		printf("server fd: %d\n", fd);

		pollfd[i].fd = fd;
		pollfd[i].events = POLLIN;
		pollfd[i].revents = 0;

		fds[i] = fd;

		if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
			flags = 0;
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	}

	if (write(wfd, &pollfd[0], sizeof(struct pollfd) * lwp) !=
	    sizeof(struct pollfd) * lwp) {
		perror("failed to write all pollfds");
		close (wfd);
		free(pollfd);
		exit(-1);
	}

	(void) time(&start_time);
	(void) time(&current_time);

	while ((int)(current_time - start_time) < run_time) {
		/*
		 * read from the devpoll driver
		 */
		dopoll.dp_timeout = -1;
		dopoll.dp_nfds = 1;
		dopoll.dp_fds = pollfd;
		result = ioctl(wfd, DP_POLL, &dopoll);
		if (debug >= 2)
			printf("result = %d\n", result);
		if (result < 0) {
			perror("/dev/poll ioctl DP_POLL failed");
			close (wfd);
			free(pollfd);
			exit(-1);
		}
		for (i = 0; i < result; i++) {
			read(dopoll.dp_fds[0].fd, buffer, BUFFER_SIZE);
		}

		(void) time(&current_time);
	}

	for (i = 0; i < lwp; i ++)
		close(fds[i]);
	close(wfd);
	free(pollfd);
	close(server);
}

void
client_corrupt(char *port, int lwp, int v6, int run_time,
    char *server_name)
{
	int i;
	struct addrinfo hints, *res = NULL;
	struct addrinfo *ressave = NULL, *bind_ressave = NULL;
	client_arg_t c_arg;
	struct sigaction act, oact;
	unsigned char buffer[BUFFER_SIZE];
	time_t start_time;		/* the start time */
	time_t current_time;		/* the current time */
	int fds[MAX_FD];

	int wfdMAX_THREAD;
	struct pollfd *pollfd;
	struct dvpoll dopoll;
	int result;
	int flags;
	int fd;
	int wfd;

	if ((wfd = open("/dev/poll", O_RDWR)) < 0) {
		exit(-1);
	}
	pollfd = (struct pollfd* )malloc(sizeof(struct pollfd) * lwp);
	if (pollfd == NULL) {
		close(wfd);
		exit(-1);
	}

	printf("%s client starting... pid: %d\n", version, getpid());

	bzero(&hints, sizeof (hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags |= AI_CANONNAME;
	hints.ai_protocol = PROTO_TEST;

	if (getaddrinfo(server_name, port, &hints, &res) != 0) {
		perror("client server_name getaddrinfo() error");
		exit(1);
	}

	while (res != NULL) {
		if (v6 == 4 && res->ai_family == AF_INET) {
			ressave = res;
			break;
		}
		if (v6 == 6 && res->ai_family == AF_INET6) {
			ressave = res;
			break;
		}
		res = res->ai_next;
	}

	if (ressave == NULL) {
		printf("please specify correct hostname\n");
		exit(1);
	}

	if (bind_name != NULL) {
		if (getaddrinfo(bind_name, NULL, &hints, &res) != 0) {
			perror("client bind_name getaddrinfo() error");
			exit(1);
		}

		while (res != NULL) {
			if (v6 == 4 && res->ai_family == AF_INET) {
				bind_ressave = res;
				break;
			}
			if (v6 == 6 && res->ai_family == AF_INET6) {
				bind_ressave = res;
				break;
			}
			res = res->ai_next;
		}

		if (bind_ressave == NULL) {
			printf("please specify correct hostname for bind()\n");
			exit(1);
		}
	}

	for (i = 0; i < lwp; i ++) {
		c_arg.c_res = ressave;
		c_arg.c_bind_res = bind_ressave;

		fd = sdp_client_connect(&c_arg);

		pollfd[i].fd = fd;
		pollfd[i].events = POLLOUT;
		pollfd[i].revents = 0;

		fds[i] = fd;

		if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
			flags = 0;
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	}

	if (write(wfd, &pollfd[0], sizeof(struct pollfd) * lwp) !=
	    sizeof(struct pollfd) * lwp) {
		perror("failed to write all pollfds");
		close (wfd);
		free(pollfd);
		exit(-1);
	}

	(void) time(&start_time);
	(void) time(&current_time);

	while ((int)(current_time - start_time) < run_time) {
		/*
		 * read from the devpoll driver
		 */
		dopoll.dp_timeout = -1;
		dopoll.dp_nfds = 1;
		dopoll.dp_fds = pollfd;
		result = ioctl(wfd, DP_POLL, &dopoll);
		if (debug >= 2)
			printf("result = %d\n", result);
		if (result < 0) {
			perror("/dev/poll ioctl DP_POLL failed");
			close (wfd);
			free(pollfd);
			exit(-1);
		}
		for (i = 0; i < result; i++) {
			write(dopoll.dp_fds[0].fd, buffer, BUFFER_SIZE);
		}

		(void) time(&current_time);
	}

	for (i = 0; i < lwp; i ++)
		close(fds[i]);
	close(wfd);
	free(pollfd);
}

int
main(int argc, char *argv[])
{
	int c;

	boolean_t client_flag = B_FALSE;
	boolean_t server_flag = B_FALSE;

	int v6 = 4;	/* IPv4 or IPv6 */

	/* the client must specify the server name or IP it will connect to */
	char *server_name = "localhost";
	/* how many threads will be created by the server or client */
	int lwp = 1;

	extern char *optarg;

	while ((c = getopt(argc, argv, "b:c:dhl:n:p:st:v:x:")) != -1) {
		switch (c) {
			case 'b':
				bind_name = optarg;
				break;
			case 'c':
				client_flag = B_TRUE;
				server_name = optarg;
				break;
			case 'h':
				usage();
				break;
			case 'l':
				sscanf(optarg, "%d", &lwp);
				break;
			case 'p':
				port = optarg;
				break;
			case 's':
				server_flag = B_TRUE;
				break;
			case 't':
				sscanf(optarg, "%d", &run_time);
				break;
			case 'v':
				sscanf(optarg, "%d", &v6);
				if (v6 == 6) {
					version = "IPv6";
				}
				break;
			case 'x':
				sscanf(optarg, "%d", &debug);
				break;
			case '?':
				usage();
				break;
			default:
				exit(1);
		}
	}

	if (lwp > MAX_FD) {
		printf("lwp > %d\n", MAX_FD);
		exit(1);
	}

	if (client_flag == B_FALSE && server_flag == B_FALSE) {
		printf("Please specify the server or the client\n");
		exit(1);
	}

	if (client_flag == B_TRUE && server_flag == B_TRUE) {
		printf("Please don't specify both server and client\n");
		exit(1);
	}

	if (server_flag == B_TRUE) {
		server_corrupt(port, lwp, v6);
	}

	if (client_flag == B_TRUE) {
		client_corrupt(port, lwp, v6, run_time,
		    server_name);
	}
}
