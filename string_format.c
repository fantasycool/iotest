#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

// stdio 实用io库
// sys/types 基本数据类型
// sys/stat 文件状态
// 符号常量

void make_temp(char *template);

int main(){
    char good_template[] = "/tmp/dirXXXXXX";
    char *wrong_template = "/tmp/dirXXXXXX";

    printf("trying to create first temp file.... \n");
    make_temp(good_template);
    printf("trying to create second temp file.... \n");
    make_temp(wrong_template);
    return 0;
}

void make_temp(char *template){
    int fd;
    struct stat sbuf;

    if((fd = mkstemp(template)) < 0){
        printf("can't create temp file \n");
        exit(0);
    }
    // 此时文件名称已经被修改了
    printf("file name is:%s \n", template);
    // 这里已经关闭，后面的unlink就等于可以直接删除了
    close(fd);

    errno = 0;
    if(stat(template, &sbuf)){
        if(errno == ENOENT){
            printf("file doesn't exits! \n");
        }else{
            printf("stat failed! \n");
        }
    } else{
        printf("file exists! \n");
        unlink(template);
    }
}