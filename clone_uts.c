#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

int fn(void *arg){
    char *command = "/bin/sh";
    // 调用一次从不返回,只有调用失败才会返回
    if(execl(command, NULL) < 0){
        printf("%s Command not found!\n", command);
        exit(0);
    }
}

int main(){
    void *pchild_stack = malloc(1024*1024);
    if(pchild_stack == NULL){
        exit(-1);
    }
    
    int pid = clone(fn, pchild_stack + 1024*1024, SIGCHLD|CLONE_NEWUTS, NULL);
    if(pid < 0){
        exit(-1);
    }
    wait(NULL);
    printf("shell进程退出了..");
    free(pchild_stack);
    printf("Child process has exited!");
}