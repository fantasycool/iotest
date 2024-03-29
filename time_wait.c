#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void){
    int err;
    struct timespec tout;
    struct tm tmp;
    
    char buf[64];
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock);
    printf("mutex is locked! \n");

    clock_gettime(CLOCK_REALTIME, &tout);
    localtime_r(&tout.tv_sec, &tmp);
    strftime(buf, sizeof(buf), "%r", &tmp);
    printf("Current time is %s \n", buf);

    tout.tv_sec += 10;
    err = pthread_mutex_timedlock(&lock, &tout);
    clock_gettime(CLOCK_REALTIME, &tout);
    localtime_r(&tout.tv_sec, &tmp);
    strftime(buf, sizeof(buf), "%r", &tmp);

    printf("the time is now %s \n", buf);
    if(err == 0){
        printf("mutex locked again! \n");
    }else{
        printf("can't lock mvtex again:%s \n", strerror(err));
    }
}