#include <stdio.h>

int main(int argc, char *argv[])
{
	int a = 0x10;
	int b = 0x01;

	printf("a: %x\n", a);
	printf("b: %x\n", b);

	a ^= b;
	b ^= a;
	a ^= b;

	printf("a: %x\n", a);
	printf("b: %x\n", b);

	printf("%x\n", (1 << 3) & ~(1ul));

	return 0;
}
