#include <stdio.h>  
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))  
#define BITS_PER_BYTE           8  
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))  
  
main()  
{  
        int j = 32;   
        printf("The size of long is %ld\n", sizeof(long));  
        printf("The convertion result is %ld\n", BITS_TO_LONGS(j));  
        printf("The convertion result is %ld\n", BITS_TO_LONGS(64));  
        printf("The convertion result is %ld\n", BITS_TO_LONGS(65));  
}
