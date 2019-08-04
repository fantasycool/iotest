#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

sigjmp_buf buf;

void handler(int sig){
    siglongjmp(buf, 1);
}

int main(){
    if(!sigsetjmp(buf, 1)){
        signal(SIGINT, handler);
        fputs("starting..\n", stdout);
    }else{
        fputs("restarting\n", stdout);
    }

    while(1){
        sleep(1);
        fputs("Processing...\n",stdout);
    }
    exit(0);
}