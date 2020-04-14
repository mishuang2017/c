/*
 * Copyright (c) 2011, 2012, Oracle and/or its affiliates. All rights reserved.
 */

/*
 * This is a client/server socket C program.
 * The client sends 65540 bytes buffer to the server. The last
 * four bytes is the crc of the previous 65536 bytes. After server
 * receives the buffer, it will re-calculate the crc to see if
 * it is correct.
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

#define	PROTO_TEST	IPPROTO_TCP

#define	MAX_THREAD_CNT	1000
#define	BUFFER_SIZE	(1)

extern int errno;

uint64_t no = 0;
uint64_t no_server = 0;

char a = '0';

/*
 * If debug == 2, the crc will be written to file for checking.
 * The log file name is /tmp/server/xxxx or /tmp/client/xxxx,
 * xxxx is the thread number
 * Don't start multiple processes when debug == 2
 */
int debug = 1;

int only = 1;

int error = 0;
int error_server = 0;

/*
 * In DR test cases, we need to test two scenarios.
 *
 * 1. When DR happens, the SDP server and SDP client exit.
 * 2. When DR happens, the SDP server and SDP client do not exit and
 *    do not close the sockets.
 */

/* the default behavior is scenario 1 */
int delay = 0;

/* the default port number */
char *port = "4000";
char *bind_name = NULL;
char *version = "IPv4";

typedef struct server_arg {
	int fd;
	char *filename;
	int thread_no;
} server_arg_t;

typedef struct client_arg {
	int run_time;
	int packets_send;
	char *filename;
	struct addrinfo *c_res;
	struct addrinfo *c_bind_res;
	int thread_no;
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
	    "\t-x	debug level, 1 or 2, default value is 0\n"
	    "\nserver usage: <b:hl:p:sv:x:>\n"
	    "\t-b	specify hostname to bind, default value is NULL\n"
	    "\t-h	print this usage\n"
	    "\t-l	lwp/thread number to be created, default value is 1\n"
	    "\t-p	port number, default value is 4000\n"
	    "\t-s	server\n"
	    "\t-v	IPv4 or IPv6, 4 or 6, default value is 4\n"
	    "\t-x	debug level, 1 or 2, default value is 0\n");

	exit(1);
}

int
reliable_write(int fd, unsigned char *buffer, int buffer_size)
{
	int ret;			/* bytes have been sent by one call */

	sleep(2);
	while (1) {
		ret = send(fd, buffer, buffer_size, MSG_NOSIGNAL);

		if (ret < 0) {
			error++;
			if (debug >= 1) {
				printf("send error: %d, ret: %d\n", errno, ret);
			}
			sleep(1);
			continue;
		}

		return (ret);
	}
}

int
reliable_read(int fd, unsigned char *buffer, int buffer_size)
{
	int ret;

	ret = read(fd, buffer, buffer_size);
	if (ret < 0 && debug >= 1)
		printf("%s: errno: %d, ret: %d\n", __func__, errno, ret);
	return (ret);
}

static unsigned int
sdp_send_buffer(int client,  int size, char *filename, unsigned char fileascii)
{
	int ret = 0, sent = 0;
	int i = 0;
	int count, index;
	FILE *out_file;
	unsigned char buffer[BUFFER_SIZE];
	memset(buffer, a, BUFFER_SIZE);
	a = (a++) % 0xff;

	unsigned int crc;
	crc = 0xFFFFFFFF;

	if (debug >= 2) {
		out_file = fopen(filename, "a");

		if (out_file == NULL) {
			(void) fprintf(stderr, "Can not open output file\n");
			exit(8);
		}
	}

	ret = reliable_write(client, buffer, BUFFER_SIZE);
	if (ret == -1)
		return (-1);
	if (debug >= 2) {
		fwrite(&crc, 1, sizeof (crc), out_file);
		fclose(out_file);
	}

	return (0);
}

void
*sdp_server_thread(void *arg)
{
	int loop = 0;
	int fd;
	int thread_no;
	server_arg_t *s_arg;
	char *filename;
	int i;
	int first_crc = 1;

	int index = 0, chars_read, chars_total = 0;
	FILE *in_file;
	unsigned char buffer[1];
	unsigned int crc, crc_read;

	s_arg = (server_arg_t *)arg;
	fd = s_arg->fd;
	thread_no = s_arg->thread_no;
	filename = s_arg->filename;

	printf("server thread: %-4d port: %s\n", thread_no, port);

	if (debug >= 2) {
		in_file = fopen(filename, "a");

		if (in_file == NULL) {
			(void) fprintf(stderr,
			    "FAIL:Can not open file for reading\n");
			exit(8);
		}
	}

	while (1) {
		chars_read = reliable_read(fd, buffer, 1);

		if (chars_read < 0) {
			sleep(1);
			error_server++;
			continue;
			no_server++;
		}

		if (!only) {
			sdp_send_buffer(fd, 1, NULL, 'a');
			no_server++;
		}
	}

	printf("error: %d\n", error_server);
	printf("no: %lld\n", no_server);
}

static void
run_corrupt(int fd, client_arg_t *c_arg)
{
	int size;			/* the current buffer size */
	unsigned char fileascii;	/* the current char to be sent */
	float  num_files;		/* counter of number of files written */
	unsigned int wrote_crc;		/* the crc to be sent */
	time_t start_time;		/* the start time */
	time_t current_time;		/* the current time */
	int ret;			/* the return value */
	int loop = 0;			/* the packets has been sent */
	char *filename;			/* the log file name /tmp/server/xxxx */

	int run_time;
	int packets_send;
	int thread_no;

	filename = c_arg->filename;
	run_time = c_arg->run_time;
	packets_send = c_arg->packets_send;
	thread_no = c_arg->thread_no;

	unsigned char buffer[1];

	(void) time(&start_time);
	(void) time(&current_time);

	fileascii = 0;

	if (debug >= 1) {
		printf("Start time is %d\n", (int)start_time);
		printf("run_time = %d\t packets_send = %d\n", run_time,
		    packets_send);
		printf("Client starting with log_file %s\n", filename);
	}

	printf("client thread: %-4d port: %s\n", thread_no, port);
	while ((int)(current_time - start_time) < run_time) {

		ret = sdp_send_buffer(fd, 1, filename, fileascii);

		if (debug >= 1) {
			printf("%d: write crc = %x\n", loop ++, ret);
		}

		if (packets_send != 0)
			if (loop == packets_send) {
				if (debug >= 1) {
					printf("%d: break, ret: %d\n", loop, ret);
				}
				break;
			}
		no++;

		if (!only) {
			reliable_read(fd, buffer, 1);
			no++;
		}

		(void) time(&current_time);

	}

	if (debug >= 1) {
		(void) printf("End time is %d\n", (int)current_time);
	}

	printf("error: %d\n", error);
	printf("run_time: %d\n", run_time);
	printf("no: %lld\n", no);
	printf("latency %f\n", ((double)run_time) * 1000000 / no);
	close(fd);
}

void
*sdp_client_thread(void *arg)
{
	int i, fd;
	struct addrinfo *ai;
	client_arg_t *c_arg;
	int nodelay = 1;
	int result;

	c_arg = (client_arg_t *)arg;

	ai = c_arg->c_res;
	if ((fd = socket(ai->ai_family, ai->ai_socktype, PROTO_TEST)) == -1) {
		perror("client socket() error");
		return (NULL);
	}

	if (!delay) {
		result = setsockopt(fd,            /* socket affected */
		    IPPROTO_TCP,     /* set option at TCP level */
		    TCP_NODELAY,     /* name of option */
		    (char *) &nodelay,  /* the cast is historical cruft */
		    sizeof(int));    /* length of option value */
		if (result < 0) {
			perror("client tcp_nodelay error");
			exit(1);
		}
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

	run_corrupt(fd, c_arg);
}

void
server_corrupt(char *port, int lwp, int v6)
{
	int i, fd, server, reuseaddr = 1;
	unsigned int addrlen;
	char *log_dir_server = "/tmp/server";
	char *log_file;
	struct sockaddr_storage cliaddr;
	struct addrinfo hints, *res = NULL, *ressave = NULL;
	pthread_t t_server[MAX_THREAD_CNT];
	server_arg_t *s_arg;
	int nodelay = 1;
	int result;

	printf("%s server starting... pid: %d\n", version, getpid());
	bzero(&hints, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

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

	if (!delay) {
		result = setsockopt(server,            /* socket affected */
		    IPPROTO_TCP,     /* set option at TCP level */
		    TCP_NODELAY,     /* name of option */
		    (char *) &nodelay,  /* the cast is historical cruft */
		    sizeof(int));    /* length of option value */
		if (result < 0) {
			perror("server tcp_nodelay error");
			exit(1);
		}
	}


	if (bind(server, ressave->ai_addr, ressave->ai_addrlen) == -1) {
		perror("server bind() error");
		exit(1);
	}

	if (listen(server, 50) == -1) {
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

		log_file = malloc(100);
		mkdir(log_dir_server, S_IRWXU | S_IRWXG | S_IRWXO);
		sprintf(log_file, "%s/%0.4d", log_dir_server, i);
		remove(log_file);
		s_arg = malloc(sizeof (struct server_arg));
		s_arg->fd = fd;
		s_arg->thread_no = i + 1;
		s_arg->filename = log_file;

		if ((pthread_create(&t_server[i], NULL,
		    sdp_server_thread, (void *)s_arg)) != 0) {
			perror("pthread_create() error");
			exit(1);
		}
	}

	for (i = 0; i < lwp; i ++) {
		pthread_join(t_server[i], NULL);
	}

	close(server);
}

void
client_corrupt(char *port, int lwp, int v6, int packets_send, int run_time,
    char *server_name)
{
	int i;
	char *log_dir_client = "/tmp/client";
	char *log_file;
	struct addrinfo hints, *res = NULL;
	struct addrinfo *ressave = NULL, *bind_ressave = NULL;
	pthread_t t_client[MAX_THREAD_CNT];
	client_arg_t *c_arg;
	struct sigaction act, oact;

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

		c_arg = malloc(sizeof (struct client_arg));
		log_file = malloc(100);
		sprintf(log_file, "%s/%0.4d", log_dir_client, i);

		c_arg->filename = log_file;
		c_arg->run_time = run_time;
		c_arg->c_res = ressave;
		c_arg->c_bind_res = bind_ressave;
		c_arg->thread_no = i + 1;
		c_arg->packets_send = packets_send;

		if (debug >= 1) {
			printf("run_time = %d\tpackets_send = %d\n",
			    run_time, packets_send);
		}

		if (debug >= 2) {
			mkdir(log_dir_client, S_IRWXU | S_IRWXG |
			    S_IRWXO);
			remove(log_file);
		}

		if ((pthread_create(&t_client[i], NULL,
		    sdp_client_thread, (void*)c_arg)) != 0) {
			perror("pthread_create() error");
			exit(1);
		}
	}

	for (i = 0; i < lwp; i ++) {
		pthread_join(t_client[i], NULL);
	}
}

int
main(int argc, char *argv[])
{
	int c;

	int client_flag = 0;
	int server_flag = 0;

	int v6 = 4;	/* IPv4 or IPv6 */

	/* packets_send is used for debugging */
	int packets_send = 0;
	/* run_time takes effect only if packets_send == 0 */
	int run_time = 5;
	/* the client must specify the server name or IP it will connect to */
	char *server_name = "localhost";
	/* how many threads will be created by the server or client */
	int lwp = 1;

	extern char *optarg;

	while ((c = getopt(argc, argv, "b:c:dhl:n:op:st:v:x:")) != -1) {
		switch (c) {
			case 'b':
				bind_name = optarg;
				break;
			case 'c':
				client_flag = 1;
				server_name = optarg;
				break;
			case 'd':
				delay = 1;
				break;
			case 'h':
				usage();
				break;
			case 'l':
				sscanf(optarg, "%d", &lwp);
				break;
			case 'n':
				sscanf(optarg, "%d", &packets_send);
				break;
			case 'p':
				port = optarg;
				break;
			case 's':
				server_flag = 1;
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
			case 'o':
				only = 1;
				break;
			case '?':
				usage();
				break;
			default:
				exit(1);
		}
	}

	if (lwp > MAX_THREAD_CNT) {
		printf("lwp > %d\n", MAX_THREAD_CNT);
		exit(1);
	}

	if (client_flag == 0 && server_flag == 0) {
		printf("Please specify the server or the client\n");
		exit(1);
	}

	if (client_flag == 1 && server_flag == 1) {
		printf("Please don't specify both server and client\n");
		exit(1);
	}

	if (server_flag == 1) {
		server_corrupt(port, lwp, v6);
	}

	if (client_flag == 1) {
		client_corrupt(port, lwp, v6, packets_send, run_time,
		    server_name);
	}
}
