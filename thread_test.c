#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void cleanup(void *arg)
{
    printf("cleanup: %s\n", (char *)arg);
}

void *thr_fn1(void *arg)
{
    printf("thread1 start \n");
    pthread_cleanup_push(cleanup, "thread1 first handler");
    pthread_cleanup_push(cleanup, "thread1 second handler");
    if (arg)
    {
        return ((void *)1);
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    return ((void *)1);
}

void *thr_fn2(void *arg)
{
    printf("thread2 start \n");
    pthread_cleanup_push(cleanup, "thread2 first handler");
    pthread_cleanup_push(cleanup, "thread2 second handler");
    if (arg)
    {
        pthread_exit((void *)2);
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    pthread_exit((void *)2);
}

int main(void)
{
    int err;
    pthread_t tid1, tid2;
    void *tret;

    err = pthread_create(&tid1, NULL, thr_fn1, (void *)1);
    if (err != 0)
    {
        printf("can't create thread1");
        exit(err);
    }
    err = pthread_create(&tid2, NULL, thr_fn2, (void *)2);
    if (err != 0)
    {
        printf("can't create thread2");
        exit(err);
    }
    // after detach, do join operation
    pthread_detach(tid1);
    err = pthread_join(tid1, &tret);
    if (err != 0)
    {
        printf("main thread join thread1 failed");
        exit(err);
    }
    printf("thread1 exit code is %ld \n", (unsigned long)tret);
    err = pthread_join(tid2, &tret);
    if (err != 0)
    {
        printf("main thread join thread2 failed");
        exit(err);
    }
    printf("thread2 exit code is %ld \n", (unsigned long)tret);
    exit(0);
}