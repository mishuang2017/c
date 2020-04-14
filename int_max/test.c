#include <stdio.h>
#include <limits.h>


enum mlx5e_vlan_rule_type {
        MLX5E_VLAN_RULE_TYPE_UNTAGGED,
        MLX5E_VLAN_RULE_TYPE_ANY_CTAG_VID,
        MLX5E_VLAN_RULE_TYPE_ANY_STAG_VID,
        MLX5E_VLAN_RULE_TYPE_MATCH_VID,
};

int b = 0;

void f(unsigned long *a)
{
	if (a)
		b = *a;
}

int main(int argc, char *argv[])
{
	unsigned long a = 10;
	printf("hello, test, %d\n", b);
/* 	f(&a); */
	f(NULL);
	printf("hello, test INI_MIN, %x\n", INT_MIN);
	printf("hello, test INT_MAX, %x\n", INT_MAX);
	printf("hello, test UINT_MAX, %x\n", UINT_MAX);
	printf("hello, test UINT_MAX, %x\n", 0 - 1);
	printf("hello, test UINT_MAX, %x\n", ~0);

	return 0;
}
