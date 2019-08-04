#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#define N 2

int main(void){
    int status, i;
    pid_t pid;

    for(i = 0; i < N; i ++){
        if((pid = fork()) == 0) {
            exit(100 + i);
        }
    }

    while((pid = waitpid(-1, &status, 0)) > 0){
        if(WIFEXITED(status)){
            printf("parent wait one child process, status is:%d, pid is:%d \n", 
                WEXITSTATUS(status), pid);
        }else{
            printf("child %d terminated abnormally \n", pid);
        }
    }

    if(errno != ECHILD){
        printf("wait failed:%d \n", errno);
        exit(errno);
    }

    exit(0);
}