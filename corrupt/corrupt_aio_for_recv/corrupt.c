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
#include <sys/devpoll.h>

#ifdef SDP
#define	PROTO_TEST	PROTO_SDP
#else
#define	PROTO_TEST	IPPROTO_TCP
#endif

#define	MAX_THREAD_CNT	1000
#define	BUFFER_SIZE	65536

#define	MAXBUF	1000

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
boolean_t dr = B_FALSE;

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

static unsigned int crc32tab[256] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


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

static unsigned int
sdp_send_buffer(int client,  int size, char *filename, unsigned char fileascii)
{
	int ret = 0, sent = 0;
	int count, index;
	FILE *out_file;
	unsigned char buffer[BUFFER_SIZE + 4];

	unsigned int crc;
	crc = 0xFFFFFFFF;

	if (debug >= 2) {
		out_file = fopen(filename, "a");

		if (out_file == NULL) {
			(void) fprintf(stderr, "Can not open output file\n");
			exit(8);
		}
	}

	/* Load the buffer with the pattern */
	for (index = 0; index < BUFFER_SIZE; index ++)
		buffer[index] = fileascii;

	for (count = 0; count < size; count ++) {
		index = count % BUFFER_SIZE;
/* 		crc = ((crc) >> 8) ^ crc32tab[fileascii ^ ((crc) & 0x000000FF)]; */
		if (index == (BUFFER_SIZE - 1)) {
			crc = htonl(crc);
			bcopy(&crc, &buffer[BUFFER_SIZE], 4);
			ret = reliable_write(client, buffer, BUFFER_SIZE + 4);
			if (ret == -1)
				return (-1);
		}
	}

	if (debug >= 2) {
		fwrite(&crc, 1, sizeof (crc), out_file);
		fclose(out_file);
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
	while ((int)(current_time - start_time) < run_time) {

		fileascii = (fileascii < 254) ? fileascii + 1 : 0;
		(void) sdp_send_buffer(fd, size, filename, fileascii);
/* 		if (thread_no % 2) */
/* 			sleep(1); */
/* 		else */
/* 			sleep(2); */

		if (debug >= 1) {
			printf("%d: write crc = %x\n", loop ++, ret);
		}

		if (ret == -1 && dr == B_FALSE) {
			break;
		}

		if (packets_send != 0)
			if (loop == packets_send)
				break;

		(void) time(&current_time);
	}

	if (debug >= 1) {
		(void) printf("End time is %d\n", (int)current_time);
	}

	close(fd);
}

void
*sdp_client_thread(void *arg)
{
	int i, fd;
	struct addrinfo *ai;
	client_arg_t *c_arg;

	c_arg = (client_arg_t *)arg;

	ai = c_arg->c_res;
	if ((fd = socket(ai->ai_family, ai->ai_socktype, PROTO_TEST)) == -1) {
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
	unsigned char buffer[BUFFER_SIZE + 4];

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

		pollfd[i].fd = fd;
		pollfd[i].events = POLLIN;
		pollfd[i].revents = 0;

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

	while (1) {

		/*
		 * read from the devpoll driver
		 */
		dopoll.dp_timeout = -1;
		dopoll.dp_nfds = lwp;
		dopoll.dp_fds = pollfd;
		result = ioctl(wfd, DP_POLL, &dopoll);
		printf("reslut = %d\n", result);
		if (result < 0) {
			perror("/dev/poll ioctl DP_POLL failed");
			close (wfd);
			free(pollfd);
			exit(-1);
		}
		for (i = 0; i < result; i++) {
			read(dopoll.dp_fds[0].fd, buffer, BUFFER_SIZE + 4);
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
	if (dr == B_TRUE) {
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

	boolean_t client_flag = B_FALSE;
	boolean_t server_flag = B_FALSE;

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

	while ((c = getopt(argc, argv, "b:c:dhl:n:p:st:v:x:")) != -1) {
		switch (c) {
			case 'b':
				bind_name = optarg;
				break;
			case 'c':
				client_flag = B_TRUE;
				server_name = optarg;
				break;
			case 'd':
				dr = B_TRUE;
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

	if (lwp > MAX_THREAD_CNT) {
		printf("lwp > %d\n", MAX_THREAD_CNT);
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
