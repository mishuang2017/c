#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/inet_diag.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>


int main(int argc, char **argv)
{
        int fd;
        struct sockaddr_nl src_addr, dest_addr;
        struct {
                struct nlmsghdr nlh;
                struct inet_diag_req r;
        } req;
        struct inet_diag_msg *pkg;
        struct msghdr msg;
        char buf[8192];
        char src_ip[20];
        char dest_ip[20];
        struct iovec iov;

        if ((fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_INET_DIAG)) < 0)
                return -1;

        int ret;
        ret = fcntl (fd, F_SETFL, O_NONBLOCK );  
        if ( ret < 0 ) {  
                fprintf ( stderr,  "Can't set socket flags");  
                close (fd);  
                return -1;  
        }
        //src addr
        memset(&src_addr, 0, sizeof(struct sockaddr_nl));
        src_addr.nl_family = AF_NETLINK;
        src_addr.nl_pid = getpid();
        src_addr.nl_groups = 0;

        if (bind(fd, (struct sockaddr *)&src_addr, sizeof(struct sockaddr_nl)) < 0) {
                fprintf(stderr, "bind socket error %s\n", strerror(errno));
        }

        memset(&req, 0, sizeof(req));
        req.nlh.nlmsg_len = sizeof(req);
        req.nlh.nlmsg_type = TCPDIAG_GETSOCK;
        req.nlh.nlmsg_flags = NLM_F_MATCH | NLM_F_REQUEST | NLM_F_ROOT;
//        req.nlh.nlmsg_flags = NLM_F_REQUEST ;
        req.nlh.nlmsg_pid = 0;

        memset(&req.r, 0, sizeof(req.r));
        req.r.idiag_family = AF_INET;
        //req.r.idiag_states = 0; //states to dump
	req.r.idiag_states = ((1 << TCP_CLOSING + 1) -1);

        //send msg to kernel
        iov.iov_base = &req;
        iov.iov_len = sizeof(req);

        msg.msg_name = (void *)&src_addr;
        msg.msg_namelen = sizeof(struct sockaddr_nl);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        if (sendmsg(fd, &msg, 0) < 0) {
                printf("%s\n", strerror(errno));
                return -1;
        }
        //recv msg from kernel
        iov.iov_base = buf;
        iov.iov_len = sizeof(buf);

        //dest addr
        memset(&src_addr, 0, sizeof(struct sockaddr_nl));
        dest_addr.nl_family = AF_NETLINK;
        dest_addr.nl_pid = 0;
        dest_addr.nl_groups = 0;

        while (1) {
                printf("while1\n");
                int status;
                struct nlmsghdr *h;

                msg = (struct msghdr) {
                        (void *)&dest_addr, sizeof(struct sockaddr_nl),
                                &iov, 1, NULL, 0, 0
                };

                //length of recv data
                status = recvmsg(fd, &msg, 0);
                //                status = recv(fd, buf, sizeof(buf), 0);       
                printf("status=%d\n", status);
                if (status < 0)
                        break;
       
                h = (struct nlmsghdr *)buf;

                while (NLMSG_OK(h, status)) {
                        printf("while2\n");
                        if (h->nlmsg_type == NLMSG_DONE) {
                                close(fd);
                                printf("NLMSG_DONE\n");
                                return 0;
                        }

                        if (h->nlmsg_type == NLMSG_ERROR) {
                                struct nlmsgerr *err;
                                err = (struct nlmsgerr*)NLMSG_DATA(h);
                                fprintf(stderr, "%d Error %d:%s\n", __LINE__, -(err->error), strerror(-(err->error)));
                                close(fd);
                                printf("NLMSG_ERROR\n");
                                return 0;
                        }

                        pkg = (struct inet_diag_msg *)NLMSG_DATA(h);
                        printf("Get a PKG..\n");
                        printf("Family:%s\n", pkg->idiag_family == AF_INET ? "AF_INET" : "AF_INET6");
                        printf("dport:%d, sprot:%d\n", pkg->id.idiag_sport, pkg->id.idiag_sport);
                        memset(src_ip, 0, sizeof(src_ip));
                        memset(dest_ip, 0, sizeof(dest_ip));
                        inet_ntop(AF_INET, pkg->id.idiag_src, src_ip, 20);
                        inet_ntop(AF_INET, pkg->id.idiag_dst, dest_ip, 20);
                        printf("%s,%s\n", src_ip, dest_ip);
                        //                get_tcp_state(pkg->idiag_state);
                        h = NLMSG_NEXT(h, status);
                        printf("status=%d\n", status);


                }//while
        }//while
        close(fd);
        return 0;
}
