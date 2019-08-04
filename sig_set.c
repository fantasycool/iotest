#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * 1. 当进城挂起的时候，我们可以通过发送 SIGINT 来进行唤醒
 * 2. 但如果将SIGINT进行BLOCK, 则SIGINT无法进行唤醒(起不到唤醒的作用了)
 * 3. 进行了UNBLOCK以后, 通过SIGINT就可以进行唤醒了
 */

void handler(int sig){
    printf("\n have received signal CTRL+C \n");
}

int main(void){
    sigset_t new_set;
    sigset_t old_set;
    
    int status;

    sigemptyset(&new_set);
    // 监听SIGINT的消息
    signal(SIGINT, handler);
    // 进程挂起，键盘发送SIGINT消息,打印CTRL+C
    printf("try to ctrl+c, check if print ctrl+c... \n");
    pause();
    printf("start to set block bit... \n");
    // 将SIGINT的消息进行阻塞
    sigaddset(&new_set, SIGINT);
    if((status = sigprocmask(SIG_BLOCK, &new_set, &old_set)) == -1){
        printf("set signal block bit failed, status is:%d \n", status);
        exit(0);
    }
    // 进城挂起，键盘发送SIGINT消息，不会打印CTRL+C
    printf("try to ctrl+c, check not print ctrl+c, only sleep...\n");
    int i;
    for(i = 0; i < 10; i++){
        sleep(1);
        printf("sleep %d seconds \n", i+1);
    }
    printf("start to remove block bit... \n");
    // 去除阻塞，查看是否会打印CTRL+C
    if((status = sigprocmask(SIG_UNBLOCK, &new_set, &old_set) == -1)){
        printf("set signal unblock bit failed, status is:%d \n", status);
        exit(0);
    }
    printf("try to ctrl+c, check if print ctrl+c...\n");
    pause();
    return 0;
}