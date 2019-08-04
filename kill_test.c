#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>

int main(void){
    pid_t pid;

    if((pid = fork()) == 0){
        pause();
        printf("Should never come here");
        exit(0);
    }
    kill(pid, SIGKILL);
    exit(0);

    printf("%d", );
}
