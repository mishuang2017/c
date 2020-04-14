#include <stdio.h>
#include <unistd.h>
#include <signal.h>

# define T 5

int flag = T;

void sigalrm_handler(int);

int  main(void)
{
    signal(SIGALRM, sigalrm_handler);   
    alarm(1);                         
	while (1);
/* 	pause(); */
}

void sigalrm_handler(int sig)
{
    if(--flag){
        printf("Hi...\n");   /*version 1*/
        /*printf("Hi...");*/ /*version 2*/
    }else{
        printf("BYE\n");
        flag=T;     
    }
    alarm(1);
}
