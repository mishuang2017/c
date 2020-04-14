#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char *name = "vxlan_sys_4000";
    char *p = name + 10;
    printf("hello, vxlan-port, %s\n", name);
    printf("hello, vxlan-port, %s\n", p);
    printf("hello, vxlan-port, %d\n", atoi(p));

    return 0;
}
