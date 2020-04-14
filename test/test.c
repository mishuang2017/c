#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

struct tc_miniflow_offload {
	bool last_flow;
	bool is_drop;
	uint32_t chain_index;
};

int main() 
{
	struct tc_miniflow_offload t, *p;
	p = (struct tc_miniflow_offload *)0x0000000400000101;
	long a;
	int i;

	t.last_flow = true;
	t.is_drop = true;
	t.chain_index = 4;
	p = &t;

	printf("%d\n", p->last_flow);
	printf("%d\n", p->is_drop);
	printf("%d\n", p->chain_index);
	for (i = 0; i < 100; i++)
		printf("%3d, %x\n", i, -i);

	return 0; 
} 
