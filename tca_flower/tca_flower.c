#include <stdio.h>

enum {
        TCA_FLOWER_UNSPEC,
        TCA_FLOWER_CLASSID,
        TCA_FLOWER_INDEV,
        TCA_FLOWER_ACT,
        TCA_FLOWER_KEY_ETH_DST,         /* ETH_ALEN */
        TCA_FLOWER_KEY_ETH_DST_MASK,    /* ETH_ALEN */
        TCA_FLOWER_KEY_ETH_SRC,         /* ETH_ALEN */
        TCA_FLOWER_KEY_ETH_SRC_MASK,    /* ETH_ALEN */
        TCA_FLOWER_KEY_ETH_TYPE,        /* be16 */
        TCA_FLOWER_KEY_IP_PROTO,        /* u8 */
        TCA_FLOWER_KEY_IPV4_SRC,        /* be32 */
        TCA_FLOWER_KEY_IPV4_SRC_MASK,   /* be32 */
        TCA_FLOWER_KEY_IPV4_DST,        /* be32 */
        TCA_FLOWER_KEY_IPV4_DST_MASK,   /* be32 */
        TCA_FLOWER_KEY_IPV6_SRC,        /* struct in6_addr */
        TCA_FLOWER_KEY_IPV6_SRC_MASK,   /* struct in6_addr */
        TCA_FLOWER_KEY_IPV6_DST,        /* struct in6_addr */
        TCA_FLOWER_KEY_IPV6_DST_MASK,   /* struct in6_addr */
        TCA_FLOWER_KEY_TCP_SRC,         /* be16 */
        TCA_FLOWER_KEY_TCP_DST,         /* be16 */
        TCA_FLOWER_KEY_UDP_SRC,         /* be16 */
        TCA_FLOWER_KEY_UDP_DST,         /* be16 */

        TCA_FLOWER_FLAGS,
        TCA_FLOWER_KEY_VLAN_ID,         /* be16 */
        TCA_FLOWER_KEY_VLAN_PRIO,       /* u8   */
        TCA_FLOWER_KEY_VLAN_ETH_TYPE,   /* be16 */

        TCA_FLOWER_KEY_ENC_KEY_ID,      /* be32 */
        TCA_FLOWER_KEY_ENC_IPV4_SRC,    /* be32 */
        TCA_FLOWER_KEY_ENC_IPV4_SRC_MASK,/* be32 */
        TCA_FLOWER_KEY_ENC_IPV4_DST,    /* be32 */
        TCA_FLOWER_KEY_ENC_IPV4_DST_MASK,/* be32 */
        TCA_FLOWER_KEY_ENC_IPV6_SRC,    /* struct in6_addr */
        TCA_FLOWER_KEY_ENC_IPV6_SRC_MASK,/* struct in6_addr */
        TCA_FLOWER_KEY_ENC_IPV6_DST,    /* struct in6_addr */
        TCA_FLOWER_KEY_ENC_IPV6_DST_MASK,/* struct in6_addr */

        TCA_FLOWER_KEY_TCP_SRC_MASK,    /* be16 */
        TCA_FLOWER_KEY_TCP_DST_MASK,    /* be16 */
        TCA_FLOWER_KEY_UDP_SRC_MASK,    /* be16 */
        TCA_FLOWER_KEY_UDP_DST_MASK,    /* be16 */
        TCA_FLOWER_KEY_SCTP_SRC_MASK,   /* be16 */
        TCA_FLOWER_KEY_SCTP_DST_MASK,   /* be16 */

        TCA_FLOWER_KEY_SCTP_SRC,        /* be16 */
        TCA_FLOWER_KEY_SCTP_DST,        /* be16 */

        TCA_FLOWER_KEY_ENC_UDP_SRC_PORT,        /* be16 */
        TCA_FLOWER_KEY_ENC_UDP_SRC_PORT_MASK,   /* be16 */
        TCA_FLOWER_KEY_ENC_UDP_DST_PORT,        /* be16 */
        TCA_FLOWER_KEY_ENC_UDP_DST_PORT_MASK,   /* be16 */

        TCA_FLOWER_KEY_FLAGS,           /* be32 */
        TCA_FLOWER_KEY_FLAGS_MASK,      /* be32 */

        TCA_FLOWER_KEY_ICMPV4_CODE,     /* u8 */
        TCA_FLOWER_KEY_ICMPV4_CODE_MASK,/* u8 */
        TCA_FLOWER_KEY_ICMPV4_TYPE,     /* u8 */
        TCA_FLOWER_KEY_ICMPV4_TYPE_MASK,/* u8 */
        TCA_FLOWER_KEY_ICMPV6_CODE,     /* u8 */
        TCA_FLOWER_KEY_ICMPV6_CODE_MASK,/* u8 */
        TCA_FLOWER_KEY_ICMPV6_TYPE,     /* u8 */
        TCA_FLOWER_KEY_ICMPV6_TYPE_MASK,/* u8 */

        TCA_FLOWER_KEY_ARP_SIP,         /* be32 */
        TCA_FLOWER_KEY_ARP_SIP_MASK,    /* be32 */
        TCA_FLOWER_KEY_ARP_TIP,         /* be32 */
        TCA_FLOWER_KEY_ARP_TIP_MASK,    /* be32 */
        TCA_FLOWER_KEY_ARP_OP,          /* u8 */
        TCA_FLOWER_KEY_ARP_OP_MASK,     /* u8 */
        TCA_FLOWER_KEY_ARP_SHA,         /* ETH_ALEN */
        TCA_FLOWER_KEY_ARP_SHA_MASK,    /* ETH_ALEN */
        TCA_FLOWER_KEY_ARP_THA,         /* ETH_ALEN */
        TCA_FLOWER_KEY_ARP_THA_MASK,    /* ETH_ALEN */

        TCA_FLOWER_KEY_MPLS_TTL,        /* u8 - 8 bits */
        TCA_FLOWER_KEY_MPLS_BOS,        /* u8 - 1 bit */
        TCA_FLOWER_KEY_MPLS_TC,         /* u8 - 3 bits */
        TCA_FLOWER_KEY_MPLS_LABEL,      /* be32 - 20 bits */

        TCA_FLOWER_KEY_TCP_FLAGS,       /* be16 */
        TCA_FLOWER_KEY_TCP_FLAGS_MASK,  /* be16 */

        TCA_FLOWER_KEY_IP_TOS,          /* u8 */
        TCA_FLOWER_KEY_IP_TOS_MASK,     /* u8 */
        TCA_FLOWER_KEY_IP_TTL,          /* u8 */
        TCA_FLOWER_KEY_IP_TTL_MASK,     /* u8 */

        __TCA_FLOWER_MAX,
};

int main(int argc, char *argv[])
{
	int i;

	for (i = 0; i < __TCA_FLOWER_MAX; i ++)
	printf("%x\n", i);

	

	return 0;
}
