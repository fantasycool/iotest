#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXLINE 128

int main(void)
{
    int fd[2];
    int pid;
    char line[MAXLINE];
    int n;

    if (pipe(fd) < 0)
    {
        printf("pipe error");
        exit(-1);
    }

    if ((pid = fork()) < 0)
    {
        printf("fork error");
        exit(-1);
    }
    else if (pid > 0)
    {
        close(fd[0]);
        write(fd[1], "hello world\n", 12);
    }
    else
    {
        close(fd[1]);
        n = read(fd[0], line, MAXLINE);
        write(STDERR_FILENO, line , n);
    }
}