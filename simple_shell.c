#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wait.h>
#include <unistd.h>

#define MAXLINE 1024
#define MAXARGS 128

// get global enviroment
extern char **environ;

void eval(char *cmdline);
/**
 * return is run in bg
 */
int parseline(char *buf, char *argv[]);
int builtin_command(char **arg);

int main()
{
    char buf[MAXLINE];
    while (1)
    {
        printf("> ");
        fgets((char *)buf, MAXLINE, stdin);
        if (feof(stdin))
        {
            printf("commandline is too long! \n");
            exit(0);
        }
        eval(buf);
    }
}

void eval(char *cmdline)
{
    char *argv[MAXARGS];
    char *buf = cmdline;
    int bg; /*Should job run in bg */
    pid_t pid;

    printf("source buf is %s \n", buf);
    bg = parseline(buf, argv);
    if (argv[0] == NULL)
    {
        // illegal cmdline
        return;
    }

    if (!builtin_command(argv))
    {
        if ((pid = fork()) == 0)
        {   
            printf("go in to %d \n", pid);
            if (execve(argv[0], argv, environ) < 0)
            {
                printf("%s command not found \n", argv[0]);
                exit(0);
            }
        }

        if (!bg)
        {
            int status;
            if (waitpid(pid, &status, 0) < 0)
            {
                printf("Wait error, pid is:%d, status is:%d \n", pid, status);
                exit(status);
            }
            else{
                printf("waitpid %d success \n", pid);
            }
        }
        else
        {
            printf("pid:%d, cmdline:%s \n", pid, cmdline);
        }
    }
}

int parseline(char *buf, char *argv[])
{
    char *s_buf;
    size_t s_size;
    char *delim_addr;
    int argc = 0;

    // 1.去掉buf尾部的\n符号，将其替换为' '
    buf[strlen(buf) - 1] = ' ';
    // 2.去掉头部的空格，提升buf中起始地址位置
    s_buf = buf;
    while (*s_buf == ' ')
    {
        s_buf++;
    }
    // 3.开始循环
    //     找到分隔符' '，将分隔符的值换为'\0'
    while ((delim_addr = strchr(s_buf, ' ')) != NULL)
    {
        //     当前的起始地址即为第一个参数地址
        argv[argc] = s_buf;
        //     将参数数量+1
        argc++;
        //     将buf设置为当前分割符号地址下一个不为' '的字节的地址
        *delim_addr = '\0';
        printf("arg argc:%d is %s \n", argc, s_buf);
        s_buf = ++delim_addr;
        //     循环去除多余的' '
        while (*s_buf && (*s_buf == ' '))
        {
            s_buf++;
        }
    }
    // 5.将参数数组进行封闭，argc处下标设置为NULL
    argv[argc] = NULL;
    // 4.如果参数数量为0，为异常情况，直接返回1
    if (argc == 0)
    {
        return 1;
    }
    // 6.如果最后一个参数为&符号，则表示需要后台运行符号，则单独拎出来，将参数数量-1
    if (strlen(argv[argc - 1]) == 1 && (*argv[argc - 1] == '&'))
    {
        argc--;
        argv[argc] = NULL;
        return 1;
    }
    // 7.返回bg的值
    return 0;
}

int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "quit"))
    {
        exit(0);
    }
    if (!strcmp(argv[0], "&"))
    {
        return 1;
    }
    return 0;
}
