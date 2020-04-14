#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define BIT(nr) (1UL << (nr))

#define VXLAN_DST_PORT 4789

enum {
        MLX5E_TC_INGRESS,
        MLX5E_TC_EGRESS,
};

#define STR(s)          #s 
#define CONS(a,b)   int(a##e##b) 

int main() 
{ 
     printf(STR(vck));                    // 输出字符串"vck" 
     printf("%d\n", CONS(2,3));   // 2e3 输出:2000 
     printf("%d\n", int(2e1));   // 2e3 输出:2000 
     return 0; 
} 
