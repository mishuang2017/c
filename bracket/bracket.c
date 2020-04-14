#include <stdio.h>

struct ab {
	int b;
};

int main(int argc, char *argv[])
{
	struct ab a;
	struct ab b = {};

	printf("hello, bracket: %d\n", a.b);
	printf("hello, bracket: %d\n", b.b);

	return 0;
}
