#include <stdio.h>

#define MLX5_BY_PASS_NUM_REGULAR_PRIOS 8
#define MLX5_BY_PASS_NUM_DONT_TRAP_PRIOS 8
#define MLX5_BY_PASS_NUM_MULTICAST_PRIOS 1
#define MLX5_BY_PASS_NUM_PRIOS (MLX5_BY_PASS_NUM_REGULAR_PRIOS +\
                                MLX5_BY_PASS_NUM_DONT_TRAP_PRIOS +\
                                MLX5_BY_PASS_NUM_MULTICAST_PRIOS)

#define LEFTOVERS_NUM_LEVELS 1
#define LEFTOVERS_NUM_PRIOS 1

#define BY_PASS_PRIO_NUM_LEVELS 1
#define BY_PASS_MIN_LEVEL (ETHTOOL_MIN_LEVEL + MLX5_BY_PASS_NUM_PRIOS +\
                           LEFTOVERS_NUM_PRIOS)

#define ETHTOOL_PRIO_NUM_LEVELS 1
#define ETHTOOL_NUM_PRIOS 11
#define ETHTOOL_MIN_LEVEL (KERNEL_MIN_LEVEL + ETHTOOL_NUM_PRIOS)
/* Vlan, mac, ttc, aRFS */
#define KERNEL_NIC_PRIO_NUM_LEVELS 4
#define KERNEL_NIC_NUM_PRIOS 1
/* One more level for tc */
#define KERNEL_MIN_LEVEL (KERNEL_NIC_PRIO_NUM_LEVELS + 1)

#define ANCHOR_NUM_LEVELS 1
#define ANCHOR_NUM_PRIOS 1
#define ANCHOR_MIN_LEVEL (BY_PASS_MIN_LEVEL + 1)

#define OFFLOADS_MAX_FT 1
#define OFFLOADS_NUM_PRIOS 1
#define OFFLOADS_MIN_LEVEL (ANCHOR_MIN_LEVEL + 1)

#define LAG_PRIO_NUM_LEVELS 1
#define LAG_NUM_PRIOS 1
#define LAG_MIN_LEVEL (OFFLOADS_MIN_LEVEL + 1)


int main(int argc, char *argv[])
{
	printf("hello, test, %x\n", BY_PASS_MIN_LEVEL);
	printf("hello, test, %x\n", LAG_MIN_LEVEL);
	printf("hello, test, %x\n", OFFLOADS_MIN_LEVEL);
	printf("hello, test, %x\n", KERNEL_MIN_LEVEL);
	printf("hello, test, %x\n", BY_PASS_MIN_LEVEL);
	printf("hello, test, %x\n", ANCHOR_MIN_LEVEL);

	printf("BY_PASS_PRIO_NUM_LEVELS = %x\n", BY_PASS_PRIO_NUM_LEVELS);
	printf("LAG_PRIO_NUM_LEVELS = %x\n", LAG_PRIO_NUM_LEVELS);
	printf("OFFLOADS_MAX_FT = %x\n", OFFLOADS_MAX_FT);
	printf("ETHTOOL_PRIO_NUM_LEVELS = %x\n", ETHTOOL_PRIO_NUM_LEVELS);
	printf("KERNEL_NIC_PRIO_NUM_LEVELS = %x\n", KERNEL_NIC_PRIO_NUM_LEVELS);
	printf("LEFTOVERS_NUM_LEVELS = %x\n", LEFTOVERS_NUM_LEVELS);
	printf("ANCHOR_NUM_LEVELS = %x\n", ANCHOR_NUM_LEVELS);

	printf("\n");
	printf("MLX5_BY_PASS_NUM_PRIOS = %x\n", MLX5_BY_PASS_NUM_PRIOS);
	printf("LAG_NUM_PRIOS = %x\n", LAG_NUM_PRIOS);
	printf("OFFLOADS_NUM_PRIOS = %x\n", OFFLOADS_NUM_PRIOS);
	printf("ETHTOOL_NUM_PRIOS = %x\n", ETHTOOL_NUM_PRIOS);
	printf("KERNEL_NIC_NUM_PRIOS = %x\n", KERNEL_NIC_NUM_PRIOS);
	printf("LEFTOVERS_NUM_PRIOS = %x\n", LEFTOVERS_NUM_PRIOS);
	printf("ANCHOR_NUM_PRIOS = %x\n", ANCHOR_NUM_PRIOS);

	return 0;
}
