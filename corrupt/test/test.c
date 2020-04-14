#include <stdio.h>

int main(int argc, char *argv[])
{
        int x=0x12345678;

        printf("%x\n", ((char*)&x)[1]);

        return 0;
}

