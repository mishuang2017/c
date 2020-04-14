#include <stdio.h>

int a = 0x1900010;

int main(int argc, char *argv[])
{
	if (a & 0x10)
		printf("hello, asm-if-and\n");

	return 0;
}
