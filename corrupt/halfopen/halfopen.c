/* #define _XPG4_2 */
/* #define __EXTENSIONS__ */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
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
#define BUF_SIZE	1024

char *server_name = "localhost";
void *sdp_testing_client1(void *server_name);
int half_flag = 0;

int main(int argc, char *argv[])
{
  int i, c;
  int ret;
  int server, conn = 0;
  int client[CLIENT];
  unsigned int addrlen;
  int client_flag = 0;
  int server_flag = 0;
  int reuseaddr = 1;
  struct sockaddr_in sin;
  struct sockaddr_in pin;
  pthread_t t_client[3], server_rd, server_wr;
  fd_set rd_set;
  fd_set master;
  struct timeval tv;
  char buf[BUF_SIZE];
  char string[10];
  int max;
  int num = 0;
  struct sigaction act, oact;
  int oob_flag, oob_size, oob_set;

  while ((c = getopt(argc, argv, "hsc:")) != -1)
    switch (c) {
      case 'h':
        half_flag = 1;
        break;
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

  if (client_flag == 0 && server_flag == 0) {
    perror("Please specify the server or the client");
    exit(1);
  }

  if (client_flag == 1) {
    for (i = 0; i < CLIENT; i ++) {
      sprintf(string, "%s/%d", server_name, i);

      if ((pthread_create(&t_client[i], NULL, sdp_testing_client1,
	  string)) != 0) {
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
    if ((conn = accept(server, (struct sockaddr *) &pin, &addrlen)) == -1) {
      perror("accept");
    }

    bzero(buf, sizeof(buf));
    memset(buf, 'a', 10);
    ret = write(conn, buf, 10);
    if (ret == -1) {
      perror("write");
    }
    printf("server write (%d) %s\n", ret, buf);

    printf("sleep 5 seconds\n");
    sleep(5);

    bzero(buf, sizeof(buf));
    memset(buf, 'b', 10);
    ret = write(conn, buf, 10);
    if (ret == -1) {
      perror("write");
    }
    printf("server write (%d) %s\n", ret, buf);

    close(conn);
  }
}

void *sdp_testing_client1(void *arg)
{
  int i, client, ret;
  struct sockaddr_in pin;
  struct hostent *hp;
  pthread_t client_rd, client_wr;
  int n;
  char c;
  char buf[BUF_SIZE];

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
  if (connect(client, (struct sockaddr *)&pin, sizeof(pin))
      == -1) {
    perror("client connect() error");
    exit(1);
  }
  printf("Client: socket created %d\n", htons(pin.sin_port));

  if (half_flag == 1) {
    printf("shutdown SHUT_WR\n");
    shutdown(client, SHUT_WR);
  }

  bzero(buf, sizeof(buf));
  ret = read(client, buf, 10);
  if (ret == -1) {
    perror("read");
  }
  printf("client read (%d) %s\n", ret, buf);

  bzero(buf, sizeof(buf));
  ret = read(client, buf, 10);
  if (ret == -1) {
    perror("read");
  }
  printf("client read (%d) %s\n", ret, buf);

  sleep(2);

  printf("close\n");
  close(client);
}
