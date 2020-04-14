#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char *a = "mishuang";

	printf("hello, memchr, %d\n", (uintptr_t)memchr(a, 's', sizeof (a)) - (uintptr_t)a);

	return 0;
}
