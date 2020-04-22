#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>
#include <linux/psample.h>
#include <string.h>

void print_nlmsghdr(const void *n, size_t len)
{
    int i = 0;

    printf("%04x: ", i);
    for (i = 0; i < len; i++) {
    printf("%02x " , ((char *)n)[i] & 0xff);
    if ((i+1) % 16 == 0)
        printf("\n%04x: ", i);
    }
    printf("\n");
}

int open_psample_netlink(int group)
{
    struct sockaddr_nl addr;
    int sock;

    sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (sock < 0) {
        perror("socket");
        return sock;
    }

    memset((void *) &addr, 0, sizeof (addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();

    if (bind(sock, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
        perror("bind");
        return -1;
    }

    if (setsockopt(sock, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &group, sizeof (group)) < 0) {
        perror("setsockopt");
        return -1;
    }

    return sock;
}

void read_psample_netlink(int sock)
{
    char buffer[MNL_SOCKET_BUFFER_SIZE];
    struct sockaddr_nl nladdr;
    struct genlmsghdr *ghdr;
    struct nlmsghdr *n;
    struct msghdr msg;
    struct iovec iov;
    int ret;
    int i;

    iov.iov_base = (void *) buffer;
    iov.iov_len = sizeof (buffer);
    msg.msg_name = (void *) &(nladdr);
    msg.msg_namelen = sizeof (nladdr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    ret = recvmsg(sock, &msg, 0);
    if (ret < 0) {
        perror("recvmsg");
        exit(EXIT_FAILURE);
    }
    n = (struct nlmsghdr *) &buffer;
    print_nlmsghdr(n, n->nlmsg_len);
    ghdr = (void *)n + sizeof (struct nlmsghdr);
    printf("ghdr->cmd: %d\n", ghdr->cmd);
}

static int _genl_ctrl_attr_cb(const struct nlattr *attr, void *data)
{
    const struct nlattr **tb = data;
    int ret = MNL_CB_OK;
    uint16_t type;

    if (mnl_attr_type_valid(attr, CTRL_ATTR_MAX) < 0) {
        perror("mnl_attr_type_valid");
        ret = MNL_CB_ERROR;
        goto done;
    }

    type = mnl_attr_get_type(attr);
    switch(type) {
        case CTRL_ATTR_FAMILY_NAME:
            if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0) {
                perror("mnl_attr_validate");
                ret = MNL_CB_ERROR;
                goto done;
            }
            break;
        case CTRL_ATTR_FAMILY_ID:
            if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0) {
                perror("mnl_attr_validate");
                ret = MNL_CB_ERROR;
                goto done;
            }
            break;
        default:
            break;
    }
    tb[type] = attr;

done:
    return ret;
}

struct group_info {
    uint32_t id;
    const char *name;
};

static int parse_mc_grps_cb(const struct nlattr *attr, void *data)
{
    const struct nlattr **tb = data;
    int type = mnl_attr_get_type(attr);

    if (mnl_attr_type_valid(attr, CTRL_ATTR_MCAST_GRP_MAX) < 0)
        return MNL_CB_OK;

    switch (type) {
    case CTRL_ATTR_MCAST_GRP_ID:
        if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
            return MNL_CB_ERROR;
        break;
    case CTRL_ATTR_MCAST_GRP_NAME:
        if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
            return MNL_CB_ERROR;
        break;
    }
    tb[type] = attr;
    return MNL_CB_OK;
}

static void parse_genl_mc_grps(struct nlattr *nested,
                   struct group_info *group_info)
{
    struct nlattr *pos;
    const char *name;

    mnl_attr_for_each_nested(pos, nested) {
        struct nlattr *tb[CTRL_ATTR_MCAST_GRP_MAX + 1] = {};

        mnl_attr_parse_nested(pos, parse_mc_grps_cb, tb);
        if (!tb[CTRL_ATTR_MCAST_GRP_NAME] ||
            !tb[CTRL_ATTR_MCAST_GRP_ID])
            continue;

        name = mnl_attr_get_str(tb[CTRL_ATTR_MCAST_GRP_NAME]);
        if (strcmp(name, group_info->name) != 0)
            continue;

        group_info->id = mnl_attr_get_u32(tb[CTRL_ATTR_MCAST_GRP_ID]);
    }
}

static int data_cb(const struct nlmsghdr *nlh, void *data)
{
    struct group_info *group_info = (struct group_info *) data;
    struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
        struct nlattr *tb[CTRL_ATTR_MAX+1] = {};

        mnl_attr_parse(nlh, sizeof (*genl), _genl_ctrl_attr_cb, tb);
        if (tb[CTRL_ATTR_MCAST_GROUPS])
        parse_genl_mc_grps(tb[CTRL_ATTR_MCAST_GROUPS], group_info);
}

int main(int argc, char *argv[])
{
    struct group_info group_info = {
        .name = PSAMPLE_NL_MCGRP_SAMPLE_NAME,
        .id = 0};
    char buf[MNL_SOCKET_BUFFER_SIZE];
    struct genlmsghdr *genl;
    struct mnl_socket *nl;
    struct nlmsghdr *nlh;
    unsigned int seq;
    int hdrsiz;
    int nls;
    int ret;

    nlh = mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type = GENL_ID_CTRL;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = seq = time(NULL);

    hdrsiz = sizeof (struct genlmsghdr);
    genl = mnl_nlmsg_put_extra_header(nlh, hdrsiz);
    genl->cmd = CTRL_CMD_GETFAMILY;
    genl->version = 1;

    mnl_attr_put_u32(nlh, CTRL_ATTR_FAMILY_ID, GENL_ID_CTRL);
    mnl_attr_put_strz(nlh , CTRL_ATTR_FAMILY_NAME, PSAMPLE_GENL_NAME) ;

    nl = mnl_socket_open(NETLINK_GENERIC);
    if (nl == NULL) {
        perror("mnl_socket_open");
        exit(EXIT_FAILURE);
    }

    if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
        perror("mnl_socket_bind");
        exit(EXIT_FAILURE);
    }

    if(mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
        perror("mnl_sockets_send");
        exit(EXIT_FAILURE);
    }

    ret = mnl_socket_recvfrom(nl, buf, sizeof (buf));
    while (ret > 0) {
        ret = mnl_cb_run(buf, ret, 0, 0, data_cb, &group_info);
        if (ret <= 0)
            break;
    }
    if (ret == -1) {
        perror("error");
        exit(EXIT_FAILURE);
    }

    mnl_socket_close(nl);

    if (!group_info.id) {
        perror("can't get psample mcast group id");
        exit(EXIT_FAILURE);
    }
    nls = open_psample_netlink(group_info.id);
    if (nls < 0)
        return nls;

    while (1)
        read_psample_netlink(nls);

    return 0;
}
