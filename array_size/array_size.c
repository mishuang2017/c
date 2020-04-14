#include <stdio.h>

struct mlx5e_tc_flow {
        long                     cookie[2];
};


int main(int argc, char *argv[])
{
        int key_len = sizeof(((struct mlx5e_tc_flow *)0)->cookie);

	printf("hello, array_size: %d\n", key_len);

	return 0;
}
