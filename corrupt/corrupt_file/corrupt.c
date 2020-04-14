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

#ifdef SDP
#define	PROTO_TEST	IPPROTO_TCP
#else
#define	PROTO_TEST	IPPROTO_TCP
#endif

#define	MAX_THREAD_CNT	1000
#define	BUFFER_SIZE	65536

char file_name[1024];
FILE *filep;

int total_bytes = 0;

extern int errno;

pthread_mutex_t clnt_mutex; /* mutex of clnt */
int g_clnt_rc = 0; /* global return code if thread is started */

/*
 * If debug == 2, the crc will be written to file for checking.
 * The log file name is /tmp/server/xxxx or /tmp/client/xxxx,
 * xxxx is the thread number
 * Don't start multiple processes when debug == 2
 */
int debug = 0;

/*
 * In DR test cases, we need to test two scenarios.
 *
 * 1. When DR happens, the SDP server and SDP client exit.
 * 2. When DR happens, the SDP server and SDP client do not exit and
 *    do not close the sockets.
 */

/* the default behavior is scenario 1 */
int dr = 0;

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
reliable_write(int fd, char *buffer, int buffer_size)
{
	int ret;			/* bytes have been sent by one call */
	int sent = 0;			/* bytes have been sent totally */
	int size = buffer_size;		/* total bytes need to be sent */
	int remain = buffer_size;	/* bytes still need to be sent */

	while (1) {
		ret = write(fd, buffer, remain);

		if (ret == 0) {
			return (ret);
		}

		if (ret == -1) {
			perror("client write() error");
			return (ret);
		}

		sent = sent + ret;
		buffer += ret;
		remain -= ret;

		if (sent != size && debug >= 1)
			printf("ret = %d\tsent = %d\tremain = %d\n",
			    ret, sent, remain);

		if (sent == size) {
			if (debug >= 1)
				printf("ret = %d\tsent = %d\tremain = %d\n",
				    ret, sent, remain);
			return (sent);
		}
	}
}

int
reliable_read(int fd, char *buffer, int buffer_size)
{
	int ret;			/* bytes have been read by one call */
	int got = 0;			/* bytes have been read totally */
	int size = buffer_size;		/* total bytes need to be read */
	int remain = buffer_size;	/* bytes still need to be read */

	while (1) {
		ret = read(fd, buffer, remain);

		if (ret == 0) {
			return (ret);
		}

		if (ret == -1) {
			perror("server read() error");
			return (got);
		}

		got = got + ret;
		buffer += ret;
		remain -= ret;

		if (got != size && debug >= 1)
			printf("ret = %d\tgot = %d\tremain = %d\n",
			    ret, got, remain);

		if (got == size) {
			if (debug >= 1)
				printf("ret = %d\tgot = %d\tremain = %d\n",
				    ret, got, remain);
			return (got);
		}
	}
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

	int index = 0, chars_read, chars_total = 0, chars_write = 0;
	FILE *in_file;
	char buffer[BUFFER_SIZE];

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
		chars_read = read(fd, buffer, BUFFER_SIZE);
		if (debug == 1)
			printf("reliable_read returns %d\n", chars_read);
		if (debug == 2)
			printf("%s\n", buffer);
		chars_write = fwrite(buffer, 1, chars_read, filep);
		total_bytes += chars_write;
		if (debug == 1)
			printf("fwrite returns %d\n", chars_write);

		if (chars_read == 0 || chars_write == 0) {
			fclose(filep);
			close(fd);
			printf("total_bytes: %d\n", total_bytes);
			return (NULL);
		}
	}
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

        size_t bytes_read;
	char buffer[BUFFER_SIZE];

	filename = c_arg->filename;
	run_time = c_arg->run_time;
	packets_send = c_arg->packets_send;
	thread_no = c_arg->thread_no;

	(void) time(&start_time);
	(void) time(&current_time);

	size = BUFFER_SIZE;
	fileascii = 0;

	if (debug >= 1) {
		printf("Start time is %d\n", (int)start_time);
		printf("run_time = %d\t packets_send = %d\n", run_time,
		    packets_send);
		printf("Client starting with log_file %s\n", filename);
	}


	printf("client thread: %-4d port: %s\n", thread_no, port);
/* 	while ((int)(current_time - start_time) < run_time) { */

	while (1) {
		if (feof(filep))
			break;
		bytes_read = fread(buffer, 1, BUFFER_SIZE, filep);
		total_bytes += bytes_read;
		if (debug == 1)
			printf("fread returns %d\n", bytes_read);
		if (debug == 2)
			printf("%s\n", buffer);
		ret = write(fd, buffer, bytes_read);
		if (debug == 1)
			printf("reliable_write returns %d\n", ret);

		if (ret == -1 && dr == 0) {
			break;
		}

		if (packets_send != 0)
			if (loop == packets_send)
				break;
	}

	if (debug >= 1) {
		(void) printf("End time is %d\n", (int)current_time);
	}

	fclose(filep);
	close(fd);

	printf("total_bytes: %d\n", total_bytes);
}

void
*sdp_client_thread(void *arg)
{
	int i, fd;
	struct addrinfo *ai;
	client_arg_t *c_arg;

	c_arg = (client_arg_t *)arg;

	ai = c_arg->c_res;
	if ((fd = socket(AF_INET_SDP, ai->ai_socktype, PROTO_TEST)) == -1) {
		pthread_mutex_lock(&clnt_mutex);
		g_clnt_rc = 1;
		pthread_mutex_unlock(&clnt_mutex);
		perror("client socket() error");
		return (NULL);
	}

	ai = c_arg->c_bind_res;
	if (ai != NULL) {
		if (bind(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
			pthread_mutex_lock(&clnt_mutex);
			g_clnt_rc = 1;
			pthread_mutex_unlock(&clnt_mutex);
			perror("client bind() error");
			return (NULL);
		}
	}

	ai = c_arg->c_res;
	if (connect(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
		pthread_mutex_lock(&clnt_mutex);
		g_clnt_rc = 1;
		pthread_mutex_unlock(&clnt_mutex);
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

	if ((server = socket(v6 == 6 ? PF_INET6 : AF_INET_SDP, SOCK_STREAM,
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

	/*
	 * On POSIX-compliant platforms, SIGPIPE is the signal sent
	 * to a process when it attempts to write to a pipe without
	 * a process connected to the other end.
	 *
	 * To prevent the process to exit, ignore this signal.
	 */
	if (dr == 1) {
		act.sa_handler = SIG_IGN;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		sigaction(SIGPIPE, &act, &oact);
	}

	if (pthread_mutex_init(&clnt_mutex, NULL) != 0) {
		perror("mutex init failed");
		exit(1);
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
			(void) pthread_mutex_destroy(&clnt_mutex);
			exit(1);
		}
	}

	for (i = 0; i < lwp; i ++) {
		pthread_join(t_client[i], NULL);
	}

	(void) pthread_mutex_destroy(&clnt_mutex);
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
	int run_time = 120;
	/* the client must specify the server name or IP it will connect to */
	char *server_name = "localhost";
	/* how many threads will be created by the server or client */
	int lwp = 1;

	extern char *optarg;

	while ((c = getopt(argc, argv, "b:c:df:hl:n:p:st:v:x:")) != -1) {
		switch (c) {
			case 'b':
				bind_name = optarg;
				break;
			case 'c':
				client_flag = 1;
				server_name = optarg;
				break;
			case 'd':
				dr = 1;
				break;
			case 'f':
				sscanf(optarg, "%s", &file_name);
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
			case '?':
				usage();
				break;
			default:
				exit(1);
		}
	}

        if (client_flag) {
                printf("file to read: %s\n", file_name);
		filep = fopen(file_name, "rb");
	}
        if (server_flag) {
                printf("file to write: %s\n", file_name);
		filep = fopen(file_name, "wb");
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

		if (g_clnt_rc == 0) {
			printf("client: PASS\n");
		} else {
			printf("client: FAIL\n");
		}

		return (g_clnt_rc);
	}
}
