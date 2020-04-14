#include <stdio.h>

void f(void)
{
        int a[2] = {1, 2};
        int b;

        b = 0x100;
        a[b] = 0x200;
}

struct as {
        long a;
        int b;
};

struct as a;

void f2(int pa, int pb)
{
        struct as *p = &a;

        p->a = pa;
        p->b = pb;
}

int main(int argc, char *argv[])
{
        printf("hello, asm\n");
        f();
        f2(1, 2);

        return 0;
}
