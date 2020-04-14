#include <stdio.h>

int main(int argc, char *argv[])
{
	int *pvar = NULL;

	typeof(*pvar) var = 999;
	printf("var:\t%d\n", var);

	return 0;
}
