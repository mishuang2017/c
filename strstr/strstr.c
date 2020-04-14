#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char *a = "0xffff88abc";
	char *s = "ffff88";

	if (strstr(a, s))
		printf("hello, strstr\n");

	return 0;
}
