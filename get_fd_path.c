#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stddef.h>

/**
 * tmpfile 应该是不会在磁盘上创建真正的文件，并且在进程退出后，这个文件就会自动被删掉
 * 经过测试tmpfile执行后，硬盘上并未出现真正的文件
 */

int main(void)
{
    FILE *file;
    struct stat s;
    int ino;
    DIR *tmp;
    struct dirent *d;

    file = tmpfile();

    fstat(fileno(file), &s);
    tmp = opendir("/tmp");
    fputs("abcdefgh\n", file);
    fflush(file);

    printf("ino is:%d \n", s.st_ino);
    ino = s.st_ino;

    long namemax;
    int len;
    struct dirent *entryp;
    namemax = pathconf("/tmp", _PC_NAME_MAX);

    if (namemax == -1)
    {
        namemax = 255;
    }
    len = offsetof(struct dirent, d_name) + namemax + 1;
    entryp = malloc(len);
    struct direct *result;
    int r_status;
    while (1)
    {
        r_status = readdir_r(tmp, entryp, &result);
        if (r_status != 0)
        {
            break;
        }
        if (result == NULL)
        {
            // 走到了最后一条
            break;
        }
        if (entryp->d_ino == ino)
        {
            printf("Have found the created filename! \n");
            printf("tmp file name /tmp/%s \n", entryp->d_name);
        }
          printf("tmp file name /tmp/%s, ino is:%d \n", entryp->d_name, entryp->d_ino);
    }
    free(entryp);
}