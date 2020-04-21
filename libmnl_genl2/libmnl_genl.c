/* This example is placed in the public domain. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>
#include <linux/psample.h>
#include <string.h>

static int group;

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

static int _genl_ctrl_attr_cb(const struct nlattr *attr, void *data)
{
    const struct nlattr **tb = data;
    uint16_t type;
    int ret = MNL_CB_OK;

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
	bool found;
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
	unsigned int id;

	mnl_attr_for_each_nested(pos, nested) {
		struct nlattr *tb[CTRL_ATTR_MCAST_GRP_MAX + 1] = {};

		mnl_attr_parse_nested(pos, parse_mc_grps_cb, tb);
		if (!tb[CTRL_ATTR_MCAST_GRP_NAME] ||
		    !tb[CTRL_ATTR_MCAST_GRP_ID])
			continue;

		name = mnl_attr_get_str(tb[CTRL_ATTR_MCAST_GRP_NAME]);
		printf("%s: name: %s\n", __func__, name);
/* 		if (strcmp(name, group_info->name) != 0) */
/* 			continue; */

		id = mnl_attr_get_u32(tb[CTRL_ATTR_MCAST_GRP_ID]);
		printf("%s: id: %d\n", __func__, id);
	}
}

static int data_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	int err;
	int ret = MNL_CB_OK;
	ssize_t payload_len;
	pid_t pid = -1;
	struct sockaddr_storage *remote_addr = NULL;

	print_nlmsghdr(nlh, nlh->nlmsg_len);
	printf("nlh->nlmsg_type: %d\n", nlh->nlmsg_type);

/* 	struct nlattr *tb[PSAMPLE_ATTR_MAX] = {}; */
        struct nlattr *tb[CTRL_ATTR_MAX+1] = {};

        err = mnl_attr_parse(nlh, sizeof(*genl), _genl_ctrl_attr_cb, tb);

        if (tb[CTRL_ATTR_FAMILY_ID]) {
            printf("family_id: %d\n", mnl_attr_get_u16(tb[CTRL_ATTR_FAMILY_ID]));
        }
        if (tb[CTRL_ATTR_FAMILY_NAME]) {
            printf("family_name: %s\n", mnl_attr_get_str(tb[CTRL_ATTR_FAMILY_NAME]));
        }
        if (tb[CTRL_ATTR_MCAST_GROUPS]) {
		struct group_info groups[3];
		parse_genl_mc_grps(tb[CTRL_ATTR_MCAST_GROUPS], groups);
        }
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	struct nlmsghdr *nlh;
	struct genlmsghdr *genl;
	int hdrsiz;
	unsigned int seq;

	char buf[MNL_SOCKET_BUFFER_SIZE];
	int ret;

	if (argc != 2) {
		printf("%s [group]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	group = atoi(argv[1]);

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = GENL_ID_CTRL;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);

	hdrsiz = sizeof (struct genlmsghdr);
	genl = mnl_nlmsg_put_extra_header(nlh, hdrsiz);
	genl->cmd = CTRL_CMD_GETFAMILY;
	genl->version = 2;

	mnl_attr_put_u32(nlh, CTRL_ATTR_FAMILY_ID, GENL_ID_CTRL);
	mnl_attr_put_strz(nlh , CTRL_ATTR_FAMILY_NAME, "psample") ;
	printf("nlh->nlmsg_len: %d\n", nlh->nlmsg_len);

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

	printf("before\n");
	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	printf("after, %d\n", ret);
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, 0, 0, data_cb, NULL);
		if (ret <= 0) {
			printf("mnl_cb_run, %d\n", ret);
			break;
		}
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
		printf("ret, %d\n", ret);
	}
	if (ret == -1) {
		perror("error");
		exit(EXIT_FAILURE);
	}

	mnl_socket_close(nl);

	return 0;
}
