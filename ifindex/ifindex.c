#include <stdio.h>
#include <net/if.h>

unsigned int if_nametoindex(const char *ifname);

char *if_indextoname(unsigned int ifindex, char *ifname);

int main(int argc, char *argv[])
{
	printf("hello, ifindex: %d\n", if_nametoindex("enp4s0f0_0"));

	return 0;
}
