#include <stdio.h>

int main(int argc, char *argv[])
{
    char *name = "vxlan_sys_4000";
    unsigned short port;
/*     sscanf(name, "vxlan_sys_%d", &port); */
/*     printf("hello, sscanf, %s\n", port); */
    char buf[10];

    sscanf("iios/12DDWDFF@122", "%*[^/]/%[^@]", buf);
    printf("hello, sscanf, %s\n", buf);

    return 0;
}
